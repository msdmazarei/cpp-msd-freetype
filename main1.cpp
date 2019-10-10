// #include "BookReader/BookAtomGroup.hpp"
// #include "BookReader/BookAtomText.hpp"
#include "BookReader/BookAtom.hpp"
#include "BookReader/BookAtomText.hpp"
#include "defs/typedefs.hpp"
#include "libs/utfcpp/source/utf8.h"
#include "renderer/book_renderer_formats.hpp"
#include "renderer/msd_renderer.hpp"
#include <codecvt>
#include <fstream>
#include <iostream>

#include <list>
#include <map>
#include <string>
#include <tuple>
#include <vector>
using namespace std;

int main() {
  // Vector<BookAtomText> v;
  // BookAtomGroup<BookAtomText> c(v);
  UTF8String s11(L"سلام");
  auto b = BookAtom(BookAtomType_Control_NewLine);
  auto sb = b.serialize_binary();
  Vector<BYTE> buf(sb->begin(), sb->end());
  cout << "Size:" << buf.size() << endl;
  auto d = buf.data(); 
    cout << endl;
  for(auto by : buf){
    std::cout << std::hex << (int) by << " " ;
  }
cout << endl;
  // auto b = UTF8toByteBuffer(L"سلام");
  int i[] = {1, 2};
  std::vector<int> a({9, 8, 3, 2});
  std::cout << a[1] << std::endl;
  std::map<int, int> items = {{1, 2}, {2, 3}, {3, 4}};
  auto s = items.find(1);
  if (s != items.end()) {
    std::cout << (s->second) << std::endl;
  } else {
    std::cout << "NOT FOUND";
  }

  UTF8String txt = L"سلاaaم";

  std::cout << "text len: " << utf8clen(txt) << std::endl;

  // read font file
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

  auto foreTextC = PixelColor(255, 255, 255, 255);
  auto backC = PixelColor(0, 0, 0, 255);
  auto selectedTextBackColor = PixelColor(0, 0, 255, 255);
  TextRenderer txtr(fontPtr, fontlength);
  UTF8String tx = L"در خانه";
  std::cout << sizeof(wchar_t) << ":len of wchar" << std::endl;
  txtr.set_font_size(32);
  auto vy = UTF8toByteBuffer(tx);
  auto vt = vy.data();
  auto l = vy.size();
  auto r = txtr.render((char *)vt, l, true);
  txtr.colorizeBitmap(r.bitmap, foreTextC, backC).write("tx.png");
  // r.bitmap.write("tx.png");
  return 0;

  return 0;
};