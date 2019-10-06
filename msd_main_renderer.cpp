#define CHAR_BIT 8
#include "msd_renderer.hpp"
// to build: g++ -I/usr/include/freetype2 -I/usr/include/freetype2/freetype
// -I/usr/include/harfbuzz -lfreetype -lharfbuzz -lpng -g msd_main_renderer.cpp
// msd_renderer.cpp -o msd_main_renderer
int main() {
  const char *fontfile;

  fontfile = "/home/msd/.fonts/Traffic.ttf";
  const char *text = "مسعود مزارعی";
  unsigned char *buffer; /*buffer*/
  FILE *fp = fopen(fontfile, "rb");
  fseek(fp, 0, SEEK_END);
  unsigned int size = ftell(fp); /*calc the size needed*/
  fseek(fp, 0, SEEK_SET);

  buffer = (unsigned char *)malloc(size); /*allocate space on heap*/

  if (fp == NULL) { /*ERROR detection if file == empty*/
    printf("Error: There was an Error reading the file");
    fclose(fp);
    free(buffer);

    return 1;
  } else if (fread(buffer, sizeof *buffer, size, fp) != size) {
    printf("problem to read full file size");
    free(buffer);
    fclose(fp);

    return 1;
  }
  fclose(fp);

  TextRenderer tx(buffer, size);
  tx.set_font_size(136);
  auto rtn = tx.render((char *)text, 8, false);
  png::rgba_pixel fc = {0, 0, 0, 255}, bc = {0, 0, 0, 0};
  auto colored = TextRenderer::colorizeBitmap(rtn.bitmap, fc, bc);
  printf("base_line:%d\n", rtn.base_line);
  colored.write("colored.png");
  free(buffer);
}