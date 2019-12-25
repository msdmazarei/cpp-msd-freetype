#include <fstream>
#include <hb-ft.h>
#include <iostream>
using namespace std;
typedef unsigned char BYTE;

int main() {
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  hb_font_t *hb_font;

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

  if ((ft_error = FT_Init_FreeType(&ft_library))) {
    std::cout << "FT_INIT_FReetype Exception";
    return -1;
  }
  if ((ft_error =
           FT_New_Memory_Face(ft_library, fontPtr, fontlength, 0, &ft_face))) {
    std::cout << ("FT_New_Memory_Face Exception");
    return -1;
  }
  auto d = get_hb_context();
  std::cout << ("hb_font creating");
  hb_font = hb_ft_font_create_referenced(ft_face);
  std::cout << ("hb_font created");
}