#pragma once

#include <optional>

#include "math/vector.hpp"
#include "shape.hpp"

// Represents an infinite plane
class Plane : public Shape {
 public:
  const Vector point;
  const Vector normal;

  Plane(const Vector& pt, const Vector& norm, const Material& mat);

  std::optional<HitInfo> intersects(const Ray& ray) const override;

  Plane* clone() const override { return new Plane(*this); }
};