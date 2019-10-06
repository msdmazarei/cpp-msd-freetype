#ifndef _MSD_RENDERER_H_
#define _MSD_RENDERER_H_
#include <hb-ft.h>
#include <png++/png.hpp>
#include <vector>

typedef unsigned char BYTE;
typedef unsigned int uint;
// typedef struct ImagePixel_ {
//   BYTE C; // R, G, B;
// } ImagePixel;

typedef struct glyph_position_ {
  hb_codepoint_t gid;
  double x, y;
  int bitmap_width;
  int bitmap_height;
  int metrics_height;
  int metrics_width;
  int metrics_horiBearingY;
  int bitmap_left;
  float x_advancing;

} glyph_position;
typedef struct TextBitmap_ {
  png::image<png::gray_pixel> bitmap;
  uint base_line;
} TextBitmap;

typedef png::image<png::rgba_pixel> RGBAImage;
typedef struct TextImage_ {
  RGBAImage image;
  uint base_line;
} TextImage;

class TextRenderer {
private:
  BYTE *font_buffer;
  unsigned long font_buffer_len;
  unsigned int font_size;

  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  hb_font_t *hb_font;
  std::vector<glyph_position> get_glyphs(char *text, unsigned int len);

  // template <typename pixel>
  // void TextRenderer::copy_to_target_image(png::image<pixel> &target_image,
  //                                         BYTE *graybitmap, uint x_offset,
  //                                         uint y_offset, uint gwidth,
  //                                         uint gheight);

  // void copy_to_target_image(ImagePixel *target, ImagePixel *glyph_bitmap,
  //                           uint16_t x_offset, uint16_t y_offset,
  //                           uint16_t gwidth, uint16_t gheight,
  //                           uint16_t image_width, uint16_t image_height);

public:
  TextRenderer(BYTE *buffer, unsigned long len);
  void set_font_size(unsigned int font_size);
  TextBitmap render(char *text, unsigned int text_len, bool ltr);
  static RGBAImage colorizeBitmap(png::image<png::gray_pixel> &src,
                                  png::rgba_pixel &foreground_color,
                                  png::rgba_pixel &background_color);
};

#endif