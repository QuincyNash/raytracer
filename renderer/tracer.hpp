#pragma once

#include "math/color.hpp"
#include "math/ray.hpp"
#include "pool.hpp"
#include "scene/scene.hpp"

// Data structure for accumulating pixel color statistics
struct PixelData {
  int samples;  // Number of samples accumulated
  Color mean;   // Current average color

  PixelData() : samples(0), mean() {}
};

struct Pixels {
  std::vector<PixelData> data;
  std::vector<bool> rowReady;
  double total_imp;

  Pixels(int w, int h) : data(w * h), rowReady(h), total_imp(0.0) {}
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

 public:
  Tracer() {}

  void refine_pixels(const Scene& scene, Pixels& pixel_data);
  void wait();

  ~Tracer() = default;

  friend class Renderer;
};