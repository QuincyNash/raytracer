#pragma once

#include <functional>
#include <iostream>

#include "math/color.hpp"
#include "math/ray.hpp"
#include "pool.hpp"
#include "scene/bvh.hpp"
#include "scene/scene.hpp"

// Data structure for accumulating pixel color statistics
struct PixelData {
  int samples;  // Number of samples accumulated
  Color mean;   // Current average color

  PixelData() : samples(0), mean() {}
};

struct Pixels {
  std::vector<PixelData> data;
  std::vector<std::atomic_bool> rowReady;

  Pixels(int w, int h) : data(w * h), rowReady(h) {}
  Pixels(const Pixels& px) = default;
  Pixels& operator=(const Pixels& other) = default;
};

// Forward declaration
class Renderer;

// Responsible for tracing rays through the scene and computing pixel colors
class Tracer {
 private:
  const Color traceRay(const Scene& scene, const Ray& ray, int depth) const;
  const Color computeLighting(const Scene& scene, const HitInfo& hitInfo,
                              int depth) const;
  ThreadPool pool{std::thread::hardware_concurrency()};
  Scene& scene;
  BVH bvh;

 public:
  Tracer(Scene& sc) : scene(sc), bvh(sc.bndedShapes) {
    std::function<void(const std::vector<BVHNode>&, int, int)> printNode =
        [&](const std::vector<BVHNode>& nodes, int index, int depth) {
          if (index < 0) return;
          const auto& n = nodes[index];
          for (int i = 0; i < depth; ++i) std::cout << "  ";
          std::cout << "Node " << index << ": " << "bounds=[" << n.bounds.min
                    << " - " << n.bounds.max << ", shapes=" << n.shapeCount
                    << "\n";
          printNode(nodes, n.left, depth + 1);
          printNode(nodes, n.right, depth + 1);
        };
    // Uncomment to print BVH structure
    // printNode(bvh.getNodes(), 0, 0);
  }

  void refinePixels(Pixels& pixel_data);
  void wait();

  ~Tracer() = default;

  friend class Renderer;
};