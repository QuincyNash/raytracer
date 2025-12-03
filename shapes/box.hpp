#pragma once

#include <optional>

#include "math/vector.hpp"
#include "shape.hpp"

// Represents a sphere in 3D space
class Box : public BoundedShape {
 public:
  const Vector min;
  const Vector max;

  Box(const Vector& bmin, const Vector& bmax, const Material& mat)
      : BoundedShape(mat, bmin, bmax), min(bmin), max(bmax) {}
  Box(const Vector& center, double width, double height, double depth,
      const Material& mat);
  std::optional<HitInfo> intersects(const Ray& ray) const override;
  Box* clone() const override { return new Box(*this); }
};