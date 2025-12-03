#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>

#include "math/camera.hpp"
#include "math/color.hpp"
#include "math/ray.hpp"
#include "math/vector.hpp"
#include "shaders/metal.hpp"
#include "shapes/plane.hpp"
#include "shapes/sphere.hpp"

void testColor() {
  std::cout << "Testing Color class..." << std::endl;

  Color c0;
  Color c1(0.5, 0.2, 0.3);
  Color c2(0.4, 0.6, 0.1);
  Color c3(255, 51, 150);

  Color c4 = c1 + c2;
  Color c5 = c1 * c2;
  Color c6 = c1 * 2.0;
  Color c7 = c3 + c3;

  assert(c0 == Color(0.0, 0.0, 0.0));
  assert(c4 == Color(0.9, 0.8, 0.4));
  assert(c5 == Color(0.2, 0.12, 0.03));
  assert(c6 == Color(1.0, 0.4, 0.6));
  assert(c7.clamp() == Color(1.0, 0.4, 1.0));

  const std::array<u_char, 3> bytes = c3.getBytes();
  assert(bytes[0] == static_cast<u_char>(255));
  assert(bytes[1] == static_cast<u_char>(51));
  assert(bytes[2] == static_cast<u_char>(150));
}

void testVector() {
  std::cout << "Testing Vector class..." << std::endl;

  Vector v1(2.0, 3.0, 1.0);
  Vector v2(1.0, 1.0, 0.0);

  Vector v3 = v1 + v2;
  Vector v4 = v1 - v2;
  Vector v5 = v1 * 2.0;
  double dotProduct = v1 * v2;
  double magnitudeV1 = v1.mag();
  double magnitudeV1Sq = v1.magSq();
  Vector v6 = v1.proj(v2);
  Vector v7 = -v1;
  Vector v8 = v1 / 0.5;
  Vector v9 = 2.0 * v1;
  Vector v10 = v1.norm();
  Vector v11 = v1.cross(v2);

  assert(v3 == Vector(3.0, 4.0, 1.0));
  assert(v4 == Vector(1.0, 2.0, 1.0));
  assert(v5 == Vector(4.0, 6.0, 2.0));
  assert(dotProduct == 5.0);
  assert(magnitudeV1Sq == 14.0);
  assert(magnitudeV1 == std::sqrt(14.0));
  assert(v6 == Vector(2.5, 2.5, 0.0));
  assert(v7 == Vector(-2.0, -3.0, -1.0));
  assert(v5 == v8 && v8 == v9);
  assert(v1 != v2);
  assert(v10.mag() == 1.0 && v1 / v1.mag() == v10);
  assert(v11 == Vector(-1.0, 1.0, -1.0));
}

void testSphereIntersect() {
  std::cout << "Testing Sphere intersection..." << std::endl;

  Material mat{};
  Sphere sphere1(Vector(0.0, 0.0, 0.0), 1.0, mat);
  Sphere sphere2(Vector(2.0, 2.0, 2.0), 0.5, mat);
  Ray ray(Vector(0.0, 0.0, -5.0), Vector(0.0, 0.0, 1.0));

  std::optional<HitInfo> hitInfoOpt1 = sphere1.intersects(ray);
  assert(hitInfoOpt1.has_value());
  HitInfo hitInfo = hitInfoOpt1.value();
  assert(std::abs(hitInfo.t - 4.0) < 1e-6);
  assert(hitInfo.pos == Vector(0.0, 0.0, -1.0));
  assert(hitInfo.normal == Vector(0.0, 0.0, -1.0));

  std::optional<HitInfo> hitInfoOpt2 = sphere2.intersects(ray);
  assert(!hitInfoOpt2.has_value());
}

void testPlaneIntersect() {
  std::cout << "Testing Plane intersection..." << std::endl;

  Material mat{};
  Plane plane1(Vector(0.0, 5.0, 0.0), Vector(0.0, 1.0, 0.0), mat);
  Plane plane2(Vector(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0), mat);
  Ray ray(Vector(0.0, -1.0, 0.0), Vector(0.0, 1.0, 0.0));

  std::optional<HitInfo> hitInfoOpt1 = plane1.intersects(ray);
  assert(hitInfoOpt1.has_value());
  HitInfo hitInfo = hitInfoOpt1.value();
  assert(std::abs(hitInfo.t - 6.0) < 1e-6);
  assert(hitInfo.pos == Vector(0.0, 5.0, 0.0));
  assert(hitInfo.normal == Vector(0.0, 1.0, 0.0));

  std::optional<HitInfo> hitInfoOpt2 = plane2.intersects(ray);
  assert(!hitInfoOpt2.has_value());
}

void testMetal() {
  std::cout << "Testing Metal integration..." << std::endl;

  MetalCompute metalCompute;
  const size_t dataSize = 1024 * 1024;
  std::vector<float> data(dataSize, 0.0f);

  metalCompute.runKernel("speedTest", data);
}

int main() {
  testColor();
  testVector();
  testSphereIntersect();
  testPlaneIntersect();
  testMetal();

  std::cout << "All tests passed!" << std::endl;

  return 0;
}