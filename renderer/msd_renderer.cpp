#include "msd_renderer.hpp"
#include "defs/typedefs.hpp"

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"
// #include <ftcolor.h>
inline void copy_glyph_bitmap_to_gray_image(png::image<png::gray_pixel> &img,
                                            FT_Bitmap *bitmap, uint x_offset,
                                            uint y_offset) {
  for (uint iy = 0; iy < bitmap->rows; iy++) {
    for (uint ix = 1; ix < bitmap->width - 1; ix++) {
      uint target_x = x_offset + ix;
      uint target_y = y_offset + iy;
      if (target_x < 0 || target_y < 0)
        continue;
      uint bufferindx = ix + iy * bitmap->width;
      img.set_pixel(target_x, target_y, *(bitmap->buffer + bufferindx));
    }
  }
}

enum FreeTypeFunction {
  func_FT_Init_FreeType = 0,
  func_FT_New_Memory_Face = 1,
  func_FT_Set_Char_Size = 2,
  func_FT_Load_Glyph = 3
};
typedef struct TextRendererException_ {
  FreeTypeFunction freetypefn;
  FT_Error error;
} TextRendererException_t;

TextRenderer::TextRenderer(BYTE *buffer, unsigned long len)
    : font_buffer(buffer), font_buffer_len(len) {
  if ((ft_error = FT_Init_FreeType(&ft_library))) {
    MLOG("FT_INIT_FReetype Exception");
    throw TextRendererException_t{func_FT_Init_FreeType, ft_error};
  }
  if ((ft_error = FT_New_Memory_Face(ft_library, buffer, len, 0, &ft_face))) {
    MLOG("FT_New_Memory_Face Exception");
    throw TextRendererException_t{func_FT_New_Memory_Face, ft_error};
  }
  MLOG("hb_font creating");
  hb_font = hb_ft_font_create(ft_face, NULL);
  MLOG("hb_font created");
}

void TextRenderer::set_font_size(unsigned int font_size) {
  if ((ft_error =
           FT_Set_Char_Size(ft_face, font_size * 64, font_size * 64, 0, 0))) {
    throw TextRendererException_t{func_FT_Set_Char_Size, ft_error};
  }
}
std::vector<glyph_position> TextRenderer::get_glyphs(char *text,
                                                     unsigned int text_len) {
  std::vector<glyph_position> rtn;
  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create();
  hb_buffer_add_utf8(hb_buffer, text, text_len, 0, -1);
  hb_buffer_guess_segment_properties(hb_buffer);

  /* Shape it! */
  hb_shape(hb_font, hb_buffer, NULL, 0);

  /* Get glyph information and positions out of the buffer. */
  unsigned int len = hb_buffer_get_length(hb_buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

  double current_x = 0;
  double current_y = 0;
  for (unsigned int i = 0; i < len; i++) {
    hb_codepoint_t gid = info[i].codepoint;
    if ((ft_error = FT_Load_Glyph(ft_face, gid, FT_LOAD_COMPUTE_METRICS | FT_LOAD_RENDER))) {
      hb_buffer_destroy(hb_buffer);
      throw TextRendererException_t{func_FT_Load_Glyph, ft_error};
    }

    double x_position = current_x + pos[i].x_offset / 64.;
    double y_position = current_y + pos[i].y_offset / 64.;
    glyph_position gp;
    gp.x = x_position;
    gp.y = y_position;
    gp.gid = gid;
    gp.bitmap_width = ft_face->glyph->bitmap.width;
    gp.bitmap_height = ft_face->glyph->bitmap.rows  ;
    MLOG2("gid:",gid);
    MLOG2("ft_face->glyph->bitmap.rows:", ft_face->glyph->bitmap.rows);
    gp.metrics_height = ft_face->glyph->metrics.height;
    gp.metrics_width = ft_face->glyph->metrics.width;
    gp.metrics_horiBearingY = ft_face->glyph->metrics.horiBearingY;
    gp.bitmap_left = ft_face->glyph->bitmap_left;
    gp.x_advancing = pos[i].x_advance / 64.;
    current_x += pos[i].x_advance / 64.;
    current_y += pos[i].y_advance / 64.;
    rtn.push_back(gp);
  }
  hb_buffer_destroy(hb_buffer);

  return rtn;
}

TextBitmap TextRenderer::render(char *text, unsigned int text_len, bool ltr) {
  MLOG("TextRenderer::render called. getting glyphs");
  auto glyphs_props = get_glyphs(text, text_len);
  int min_y = 0, max_y = 0, image_height = 0, image_width = 0;
  int min_x = 0;
  float _image_width = 0;
  for (auto g : glyphs_props) {
    MLOG2("g.gid:", g.gid);
    MLOG2("g.bitmap_h:", g.bitmap_height);
    MLOG2("g.metrics_height:", g.metrics_height);
    if (g.metrics_height == 0)
      continue;
    float conversion_ratio = (float)g.bitmap_height / (float)g.metrics_height;
    float over_base_line = g.metrics_horiBearingY * conversion_ratio;
    min_y = min_y < over_base_line - g.bitmap_height
                ? min_y
                : over_base_line - g.bitmap_height;
    max_y = max_y > over_base_line ? max_y : over_base_line;
    min_x = min_x < (g.x + g.bitmap_left) ? min_x : g.x + g.bitmap_left;
    _image_width += g.x_advancing; // + g.bitmap_left;
  }
  image_height = max_y - min_y;
  int width_by_last_element =
      glyphs_props.back().x + glyphs_props.back().bitmap_width;
  image_width = _image_width > width_by_last_element ? _image_width
                                                     : width_by_last_element;

  // image_width = glyphs_props.back().x + glyphs_props.back().bitmap_width;
  BYTE *image_buffer = (BYTE *)malloc(image_height * image_width * 1);
  unsigned int glyphs_len = glyphs_props.size();
  png::image<png::gray_pixel> img_rtn(image_width, image_height);
  MLOG2("img_rtx.w:", image_width);
  MLOG2("img_rtn.h:", image_height);

  for (unsigned int i = 0; i < glyphs_len; i++) {
    unsigned int glyph_index = ltr ? i : glyphs_len - i - 1;
    auto current_glyph = glyphs_props[glyph_index];
    if (current_glyph.metrics_height == 0)
      continue;
    MLOG("loading and rendering glyph");
    FT_Load_Glyph(ft_face, current_glyph.gid, FT_LOAD_DEFAULT);
    FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);
    MLOG("glyph rendererd");
    // char *filename = (char *)malloc(2);
    // *filename = '0' + i;
    // *(filename + 1) = 0;

    uint true_width = ft_face->glyph->bitmap.width;
    uint true_height = ft_face->glyph->bitmap.rows;
    MLOG2("glyph w:", true_width);
    MLOG2("glyph h:", true_height);
    int current_x = current_glyph.x + current_glyph.bitmap_left;
    int gy = max_y - (float)current_glyph.bitmap_height /
                         current_glyph.metrics_height *
                         current_glyph.metrics_horiBearingY;

    auto x_from = current_x < 0 ? -current_x : 0;
    MLOG2("current_x: ", current_x);
    MLOG2("gy:", gy);
    copy_glyph_bitmap_to_gray_image(img_rtn, &ft_face->glyph->bitmap,
                                    current_x < 0 ? 0 : current_x, gy);
  }
  MLOG("Atom renderered successfully");
  return {img_rtn, (uint)max_y};
}
inline BYTE getColor(float colorDistance, BYTE fColor, BYTE bColor) {
  return (BYTE)(colorDistance * fColor) + (BYTE)((1 - colorDistance) * bColor);
}
RGBAImage TextRenderer::colorizeBitmap(png::image<png::gray_pixel> &src,
                                       png::rgba_pixel &foreground_color,
                                       png::rgba_pixel &background_color) {
  RGBAImage rtn(src.get_width(), src.get_height());
  for (uint iy = 0; iy < src.get_height(); iy++)
    for (uint ix = 0; ix < src.get_width(); ix++) {
      BYTE gray_value = src.get_pixel(ix, iy);
      float colorDistance = (float)gray_value / 255;
      BYTE r =
          getColor(colorDistance, foreground_color.red, background_color.red);
      // (BYTE)(colorDistance * foreground_color.red) +
      //          (BYTE)((1 - colorDistance) * background_color.red);
      BYTE g = getColor(colorDistance, foreground_color.green,
                        background_color.green);

      // (BYTE)(colorDistance * foreground_color.green) +
      //          (BYTE)((1 - colorDistance) * background_color.green);
      BYTE b =
          getColor(colorDistance, foreground_color.blue, background_color.blue);

      // (BYTE)(colorDistance * foreground_color.blue) +
      //          (BYTE)((1 - colorDistance) * background_color.blue);
      BYTE a = getColor(colorDistance, foreground_color.alpha,
                        background_color.alpha);

      // (BYTE)(colorDistance * foreground_color.alpha) +
      //          (BYTE)((1 - colorDistance) * background_color.alpha);
      rtn.set_pixel(ix, iy, png::rgba_pixel(r, g, b, a));
    }
  return rtn;
}
