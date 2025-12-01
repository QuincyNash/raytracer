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
  double closest_t = std::numeric_limits<double>::max();

  for (const std::unique_ptr<Shape>& shape : scene.shapes) {
    std::optional<HitInfo> hitOpt = shape->intersects(ray);
    if (hitOpt.has_value() && hitOpt->t < closest_t) {
      closest_t = hitOpt->t;
      closestHit.emplace(hitOpt.value());
    }
  }

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
    for (const std::unique_ptr<Shape>& shape : scene.shapes) {
      const Vector toLight = light.position - i;
      // Offset origin slightly to avoid self-intersection
      const Ray shadowRay(i, toLight);
      std::optional<HitInfo> shadowHitOpt = shape->intersects(shadowRay);

      if (shadowHitOpt.has_value()) {
        const double distToLightSq = toLight.magSq();
        const double tSq = shadowHitOpt->t * shadowHitOpt->t;
        if (tSq < distToLightSq && shadowHitOpt->t > Shape::EPS) {
          inShadow = true;
          break;
        }
      }
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
void Tracer::refine_pixels(const Scene& scene, Pixels& pixels) {
  const int w = scene.getWidth();
  const int h = scene.getHeight();
  const int refl = scene.reflections();
  const Camera camera = scene.getCamera();

  for (int row = 0; row < h; ++row) {
    PixelData* rowData = pixels.data.data() + row * w;

    pool.enqueue([this, &pixels, scene, rowData, camera, row, w, h, refl]() {
      thread_local std::mt19937 rng(std::random_device{}());
      thread_local std::uniform_real_distribution<double> dist(-0.5, 0.5);

      std::vector<PixelData> newRowData(w);

      for (int x = 0; x < w; ++x) {
        int old_samples = rowData[x].samples;

        double x_quad = 0.5, y_quad = 0.5, x_offset = 0.0, y_offset = 0.0;
        if (old_samples > 0) {
          const int a = 4;
          x_quad = ((old_samples % a + 0.5) / 4.0);
          y_quad = (((old_samples / a) % a + 0.5) / 4.0);
          x_offset = x_quad + dist(rng) / 4.0;
          y_offset = y_quad + dist(rng) / 4.0;
        }

        Ray ray = camera.ray(x + x_offset, row + y_offset, w, h);
        Color c = traceRay(scene, ray, refl);

        Color old_total = rowData[x].mean * static_cast<double>(old_samples);
        newRowData[x].mean =
            (old_total + c) / static_cast<double>(old_samples + 1);
        newRowData[x].samples = old_samples + 1;
      }

      // Copy new row data into shared pixel buffer
      std::copy_n(newRowData.data(), w, rowData);
      pixels.rowReady[row] = true;
    });
  }
  // pool.wait_for(std::chrono::milliseconds(15));  // Wait for ~1 frame
}

void Tracer::wait() { pool.wait(); }
