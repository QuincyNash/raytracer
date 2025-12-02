#include "tracer.hpp"

#include <algorithm>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "renderer/pool.hpp"
#include "scene/scene.hpp"

// Trace a ray through the scene and return the resulting color
const Color Tracer::traceRay(const Scene& scene, const Ray& ray,
                             int depth) const {
  // Check for intersections with all shapes in the scene
  std::optional<HitInfo> closestHit;
  double closestT = __DBL_MAX__;

  // Check non-bounded shapes normally
  for (const std::unique_ptr<Shape>& shape : scene.nonBndedShapes) {
    std::optional<HitInfo> hitOpt = shape->intersects(ray);
    if (hitOpt.has_value() && hitOpt->t < closestT) {
      closestT = hitOpt->t;
      closestHit.emplace(hitOpt.value());
    }
  }

  // Check bounded shapes using BVH
  bvh.traverse(scene.bndedShapes, ray,
               [&](const BoundedShape& shape, const HitInfo& hitInfo) {
                 if (hitInfo.t < closestT) {
                   closestT = hitInfo.t;
                   closestHit.emplace(hitInfo);
                 }
               });

  if (closestHit.has_value()) {
    return computeLighting(scene, closestHit.value(), depth);
  } else {
    return scene.getBackground();
  }
}

// Compute lighting for all lights at the hit point
const Color Tracer::computeLighting(const Scene& scene, const HitInfo& hitInfo,
                                    int depth) const {
  // Convenience variables
  const Vector i = hitInfo.pos;
  const Vector d = hitInfo.ray.dir;
  const Vector n = hitInfo.normal;
  const Material* mat = hitInfo.material;

  // Ambient light contribution (doesn't depend on lights)
  const double ambFactor = scene.getAmbientLight() * (1 - mat->reflectivity);
  const Color ambient = mat->color * ambFactor;

  Color finalColor = ambient;

  for (const Light& light : scene.lights) {
    bool inShadow = false;

    // Shadow check (cast shadow ray toward light)
    for (const std::unique_ptr<Shape>& shape : scene.nonBndedShapes) {
      const Vector toLight = light.position - i;
      // Offset origin slightly to avoid self-intersection
      const Ray shadowRay(i, toLight);
      std::optional<HitInfo> shadowHitOpt = shape->intersects(shadowRay);

      if (shadowHitOpt.has_value()) {
        const double distToLightSq = toLight.magSq();
        const double tSq = shadowHitOpt->t * shadowHitOpt->t;
        if (tSq < distToLightSq && shadowHitOpt->t > Vector::EPS) {
          inShadow = true;
          break;
        }
      }
    }
    // Shadow check for bounded shapes using BVH
    if (!inShadow) {
      const Vector toLight = light.position - i;
      const Ray shadowRay(i, toLight);
      bvh.traverseFirstHit(
          scene.bndedShapes, shadowRay,
          [&](const BoundedShape& shape, const HitInfo& shadowHit) {
            const double distToLightSq = toLight.magSq();
            const double tSq = shadowHit.t * shadowHit.t;
            if (tSq < distToLightSq && shadowHit.t > Vector::EPS) {
              inShadow = true;
            }
          });
    }

    const Vector lt = (light.position - i).norm();

    // Diffuse light contribution
    Color diffuse;
    if (!inShadow) {
      const double diffFactor =
          (1 - ambFactor) * (1 - mat->reflectivity) * std::max(0.0, n * lt);
      diffuse = mat->color * light.color * diffFactor;
    }

    // Specular light contribution
    Color specular;
    if (!inShadow) {
      const Vector h = (lt - d.norm()).norm();
      specular = mat->specular * mat->specularFactor *
                 std::pow(std::max(0.0, n * h), mat->shininess) * light.color;
    }

    // Reflective contribution
    Color reflective;
    if (depth > 0 && mat->reflectivity > 0) {
      const Vector reflectDir = d - 2.0 * d.proj(n);
      const Ray ray(i, reflectDir);
      const Color reflectColor = traceRay(scene, ray, depth - 1);
      reflective = (1 - ambFactor) * mat->reflectivity * reflectColor;
    }

    // Sum contributions
    finalColor = finalColor + diffuse + specular + reflective;
  }

  return finalColor;
}

// Expects preallocated pixels vector
// Updates pixels in place by adding new rays
// If simple, performs uniform sampling with 1 ray per pixel
void Tracer::refinePixels(Pixels& pixels) {
  const int w = scene.getWidth();
  const int h = scene.getHeight();
  const int refl = scene.reflections();
  const Camera camera = scene.getCamera();

  for (int row = 0; row < h; ++row) {
    PixelData* rowData = pixels.data.data() + row * w;

    pool.enqueue([this, &pixels, rowData, camera, row, w, h, refl]() {
      thread_local std::mt19937 rng(std::random_device{}());
      thread_local std::uniform_real_distribution<double> dist(-0.5, 0.5);

      std::vector<PixelData> newRowData(w);

      for (int x = 0; x < w; ++x) {
        int oldSamples = rowData[x].samples;

        double xQuad = 0.5, yQuad = 0.5, xOffset = 0.0, yOffset = 0.0;
        if (oldSamples > 0) {
          const int a = 4;
          xQuad = ((oldSamples % a + 0.5) / 4.0);
          yQuad = (((oldSamples / a) % a + 0.5) / 4.0);
          xOffset = xQuad + dist(rng) / 4.0;
          yOffset = yQuad + dist(rng) / 4.0;
        }

        Ray ray = camera.ray(x + xOffset, row + yOffset, w, h);
        Color c = traceRay(scene, ray, refl);

        Color oldTotal = rowData[x].mean * static_cast<double>(oldSamples);
        newRowData[x].mean =
            (oldTotal + c) / static_cast<double>(oldSamples + 1);
        newRowData[x].samples = oldSamples + 1;
      }

      // Copy new row data into shared pixel buffer
      std::copy_n(newRowData.data(), w, rowData);
      pixels.rowReady[row].store(true, std::memory_order_release);
    });
  }
}

void Tracer::wait() { pool.wait(); }
