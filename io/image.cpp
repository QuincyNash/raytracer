#include "image.hpp"

#include <fstream>

#include "io/tinyfiledialogs.hpp"
#include "renderer/tracer.hpp"

// Save the image to a PPM file (return true on success)
bool Image::save() const {
  // Get filename from user from tinyfiledialogs
  const char* saveFile = tinyfd_saveFileDialog("Save file",     // title
                                               "./output.ppm",  // default name
                                               0, NULL,         // no filters
                                               "Text Files"     // description
  );
  if (!saveFile) return false;
  const std::string filename(saveFile);

  std::ofstream output(filename, std::ios::binary);
  if (!output.is_open()) {
    return false;
  }

  // PPM format:
  // P6
  // <width> <height>
  // 255
  // r g b r g b r g b ...
  output << "P6" << std::endl;  // Binary PPM
  output << scene.getWidth() << " " << scene.getHeight() << std::endl;
  output << "255" << std::endl;
  for (const PixelData& px : pixels.data) {
    output.write(reinterpret_cast<const char*>(px.mean.getBytes().data()), 3);
  }
  output.close();
  return true;
}