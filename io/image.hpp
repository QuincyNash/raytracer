#pragma once

#include <string>
#include <thread>
#include <vector>

#include "math/color.hpp"
#include "renderer/renderer.hpp"

// Handles image creation and saving to files
// Quality parameter controls number of samples per pixel
class Image {
 private:
  const Scene& scene;
  const Pixels& pixels;

 public:
  Image(const Renderer& renderer)
      : scene(renderer.scene), pixels(renderer.frontPixels) {}
  Image(const Scene& sc, int quality = 129)
      : scene(sc), pixels([&]() -> Pixels& {
          static Pixels tempPixels(sc.getWidth(), sc.getHeight());
          return tempPixels;
        }()) {
    Tracer tracer = Tracer();
    for (int i = 0; i < quality; i++) {
      tracer.refine_pixels(scene, const_cast<Pixels&>(pixels));
    }
    tracer.wait();
  }
  bool save() const;

  ~Image() = default;
};