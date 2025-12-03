#include "shapes/box.hpp"

#include <algorithm>

// Bounding box is center +/- half-dimensions in all directions
Box::Box(const Vector& center, double width, double height, double depth,
         const Material& mat)
    : BoundedShape(mat, center - Vector(width, height, depth) / 2.0,
                   center + Vector(width, height, depth) / 2.0),
      min(center - Vector(width, height, depth) / 2.0),
      max(center + Vector(width, height, depth) / 2.0) {}

std::optional<HitInfo> Box::intersects(const Ray& ray) const {
  double tmin = std::numeric_limits<double>::lowest();
  double tmax = std::numeric_limits<double>::max();

  // For each axis, compute intersection with that axis's slabs
  for (int i = 0; i < 3; ++i) {
    double invD = 1.0 / ray.dir[i];
    double t0 = (min[i] - ray.orig[i]) * invD;
    double t1 = (max[i] - ray.orig[i]) * invD;

    if (invD < 0.0) std::swap(t0, t1);

    tmin = std::max(tmin, t0);
    tmax = std::min(tmax, t1);

    if (tmax < tmin) return std::nullopt;  // No hit
  }

  double t = (tmin > Vector::EPS) ? tmin : tmax;
  if (t < Vector::EPS) return std::nullopt;

  Vector pos = ray.at(t);

  // Compute normal: check which face we hit
  Vector normal(0, 0, 0);

  if (std::abs(pos.x() - min.x()) < Vector::EPS)
    normal = Vector(-1, 0, 0);
  else if (std::abs(pos.x() - max.x()) < Vector::EPS)
    normal = Vector(1, 0, 0);
  else if (std::abs(pos.y() - min.y()) < Vector::EPS)
    normal = Vector(0, -1, 0);
  else if (std::abs(pos.y() - max.y()) < Vector::EPS)
    normal = Vector(0, 1, 0);
  else if (std::abs(pos.z() - min.z()) < Vector::EPS)
    normal = Vector(0, 0, -1);
  else if (std::abs(pos.z() - max.z()) < Vector::EPS)
    normal = Vector(0, 0, 1);

  const HitInfo hitInfo(pos, normal, ray, t, &material);
  return hitInfo;
}