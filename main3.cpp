#include <fstream>
#include <iostream>
#include <mupdf/fitz.h>

using namespace std;
typedef unsigned char BYTE;
int main() {

  ifstream ifs("/home/msd/.fonts/Zar.ttf", ios::binary | ios::ate);
  ifstream::pos_type pos = ifs.tellg();

  // What happens if the OS supports really big files.
  // It may be larger than 32 bits?
  // This will silently truncate the value/
  int fontlength = pos;

  // Manuall memory management.
  // Not a good idea use a container/.
  BYTE *fontPtr = new BYTE[fontlength];
  ifs.seekg(0, ios::beg);
  ifs.read((char *)fontPtr, fontlength);

  // No need to manually close.
  // When the stream goes out of scope it will close the file
  // automatically. Unless you are checking the close for errors
  // let the destructor do it.
  ifs.close();

  fz_context *ctx;
  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  auto buf = fz_new_buffer_from_data(ctx, fontPtr, fontlength);
  auto fnt = fz_new_font_from_buffer(ctx, NULL, buf, 0, 1);
  auto txt = fz_new_text(ctx);

  fz_matrix m;
  m.a = 100;
  m.d = 100;
  m.b = 0;
  m.c = 0;
  m.e = 0;
  m.f = 0;
  int utf8_i[10] = {216, 173, 216, 179, 219, 140, 217, 134, 0, 0};
  char *utf8_text = new char[10];
  for (int i = 0; i < 10; i++)
    utf8_text[i] = (char)utf8_i[i];
   
 auto shaper=  fz_font_shaper_data(ctx,fnt);
 
fz_pixmap *pix = fz_render_ft_glyph_pixmap(ctx,fnt,250,m,0);

 fz_color_params cps;
    fz_buffer *buf1 = fz_new_buffer(ctx, 1000000);
    // fz_new_buffer_from_pixmap_as_png(ctx, pix, fz_default_color_params);
    fz_output *fzo = fz_new_output_with_buffer(ctx, buf1);
    fz_write_pixmap_as_png(ctx, fzo, pix);
    const char *b = fz_string_from_buffer(ctx, buf1);
    size_t a = fz_buffer_storage(ctx, buf1, NULL);
    auto f = fopen("1.png","wb");
    fwrite(b,1,a,f);
    fclose(f);
return 0;

// o->write()

  fz_show_string(ctx, txt, fnt, m, utf8_text, 0, 0, FZ_BIDI_RTL, FZ_LANG_UNSET);
  fz_colorspace *fzcs =
      fz_new_colorspace(ctx, FZ_COLORSPACE_RGB, 0, 3,
                        "DeviceRGB"); // FZ_COLORSPACE_IS_DEVICE
  auto fzSeparation = fz_new_separations(ctx, 1);
  auto pixm = fz_new_pixmap(ctx, fzcs, 500, 100, fzSeparation, 0);
  pixm->colorspace = fzcs;
  
  auto drawDev = fz_new_draw_device(ctx, m, pixm);
  fz_color_params fzp;
  const float c = 0;
  fz_default_colorspaces default_cs;
  default_cs.rgb = fz_device_rgb(ctx);
  drawDev->set_default_colorspaces(ctx, drawDev, &default_cs);
  drawDev->fill_text(ctx, drawDev, txt, m, fzcs, &c, 20, fzp);

  auto m1 = fz_render_ft_glyph_pixmap(ctx, fnt, 400, m, 1);

  std::cout << "h:" << m1->h << " w: " << m1->w << std::endl;
}