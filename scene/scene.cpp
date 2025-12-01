#include "scene.hpp"

#include "light.hpp"
#include "math/color.hpp"
#include "math/vector.hpp"

// Set camera parameters
void Scene::setCamera(const Vector pos, const Vector dir,
                      const double fov_deg) {
  camera.position = pos;
  camera.setDir(dir);
  camera.fov = fov_deg;
}

void Scene::setCameraPos(const Vector pos) { camera.position = pos; }

void Scene::setCameraDir(const Vector dir) { camera.setDir(dir); }

void Scene::setCameraFov(const double fov_deg) { camera.fov = fov_deg; }

void Scene::eulerRotateCamera(int dx, int dy) { camera.eulerRotate(dx, dy); }

void Scene::moveCameraPosition(const Vector& delta) {
  camera.movePosition(delta);
}

void Scene::zoomCamera(double scroll) { camera.zoom(scroll); }

void Scene::setAmbientLight(const double ambient) { ambientLight = ambient; }

// Set background color
void Scene::setBackground(const int r, const int g, const int b) {
  background = Color(r, g, b);
}

// Add a light to the scene
void Scene::addLight(const Vector pos, const Color color) {
  lights.push_back(Light{pos, color});
}

// Add a shape to the scene
void Scene::addShape(std::unique_ptr<Shape> shape) {
  shapes.push_back(std::move(shape));
}
