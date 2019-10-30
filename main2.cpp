// compile:  g++ -I. -I ./defs -I ./renderer/ `pkg-config harfbuzz freetype2
// --cflags` -lpng -lfreetype -lharfbuzz  main.cpp BookReader/*cpp defs/*cpp
// renderer/*cpp -o main -g
#include "BookReader/Book.hpp"
#include "BookReader/BookAtomGroup.hpp"
#include "BookReader/BookAtomText.hpp"
#include "BookReader/BookDirectionGroup.hpp"
#include "libs/utfcpp/source/utf8.h"
#include "renderer/book_renderer.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

int main(int argc, char **argv) {

  auto rbook_file =
      ifstream("book1.msd", std::ios_base::binary | std::ios::ate);
  cout << rbook_file.is_open() << endl;
  auto filel = rbook_file.tellg();
  cout << filel << endl;
  BYTE *buf = new BYTE[filel];
  rbook_file.seekg(0, ios::beg);
  rbook_file.read((char *)buf, filel);
  rbook_file.close();
  for (int i = 0; i < 32; i++) {
    cout << hex << (int)buf[i] << " ";
  }
  cout << endl;
  Book *b1 = Book::deserialize((int)filel, buf);
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
  BookRendererFormat bookFormat(foreTextC, backC, foreTextC,
                                selectedTextBackColor, 30, fontPtr, fontlength);

  BookRenderer bookRenderer(b1, &bookFormat, 500, 500);
  BookPosIndicator bpi = b1->nextAtom(BookPosIndicator());
  auto poses = bookRenderer.getPageIndicators();
  int j=0;

  for (auto p:poses){
    std::cout << "G: "<< p[0] << " A: " << p[1]<< std::endl;
      auto rtn = bookRenderer.renderMsdFormatPageAtPointFW(p);
    MsdBookRendererPage *r = new MsdBookRendererPage(rtn);
    rtn.getImage().write("p_pageImage_"+to_string(j)+".png");
    j++;  
  }
  while (b1->is_last_atom(bpi) == 0) {
    std::cout << "isFisrtAtom:" << b1->is_first_atom(bpi)
              << " isLastAtom:" << b1->is_last_atom(bpi) << std::endl;
    std::cout << "G:" << (int) bpi[0] << "A:" <<(int) bpi[1] << std::endl;
    auto rtn = bookRenderer.renderMsdFormatPageAtPointFW(bpi);
    MsdBookRendererPage *r = new MsdBookRendererPage(rtn);
    auto img = rtn.getImage();
    img.m_info;
    img.write("pageImage_"+to_string(j)+".png");
    j++;  
    bpi = r->getEndPagePointer();
  }

  return 0;
}