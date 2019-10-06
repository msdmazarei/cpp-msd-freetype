#ifndef MSD_IMAGE_UTILS
#define MSD_IMAGE_UTILS

#include "../renderer/msd_renderer.hpp"
#include "defs/typedefs.hpp"
#include <png++/png.hpp>

void copyPngToPng(RGBAImage &OrigImg, WORD targetX, WORD targetY,
                  RGBAImage &ImgToCopy, WORD fromX, WORD fromY, WORD toX,
                  WORD toY) {
  for (WORD y = 0; y <= toY - fromY; y++)
    for (WORD x = 0; x <= toX - fromX; x++) {
      auto pix = ImgToCopy.get_pixel(x+fromX, y+fromY);
      OrigImg.set_pixel(x + targetX, y + targetY, pix);
    }
}

void fillImageWithPixel(RGBAImage &img, PixelColor c) {
  for (WORD y = 0; y < img.get_height(); y++)
    for (WORD x = 0; x < img.get_width(); x++)
      img.set_pixel(x, y, c);
}

#endif