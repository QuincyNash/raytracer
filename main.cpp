#include <fstream>

#include "io/image.hpp"
#include "io/tinyfiledialogs.hpp"
#include "renderer/renderer.hpp"
#include "renderer/tracer.hpp"
#include "shapes/box.hpp"
#include "shapes/plane.hpp"
#include "shapes/sphere.hpp"

int main() {
  Scene scene(512, 512, 6);

  scene.setBackground(135, 206, 235);
  scene.setCamera(Vector(0, 0, 0.5), Vector(0, 1, 0), 60.0);
  scene.setAmbientLight(0.2);
  scene.addLight(Vector(0, -0.5, 1.0), Color(255, 255, 255));

  scene.addShape(std::make_unique<Plane>(Vector(0, 0, 0), Vector(0, 0, 1),
                                         (Material){
                                             .color = Color(255, 255, 255),
                                             .reflectivity = 0.0,
                                         }));
  for (double x = -3.0; x <= 3.0; x += 0.2) {
    for (double y = 1.0; y <= 1.4; y += 0.2) {
      scene.addShape(std::make_unique<Sphere>(Vector(x, y, 0.1), 0.1,
                                              (Material){
                                                  .color = Color(255, 0, 0),
                                                  .reflectivity = 0.3,
                                              }));
    }
  }

  Renderer renderer{scene, 30};
  renderer.run();

  // Image image{scene};
  // image.save();

  return 0;
}