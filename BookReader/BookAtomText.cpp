#include "BookAtomText.hpp"

BookAtomText::BookAtomText(UTF8String str)
    : str(str), BookAtom(BookAtomType_Text){};

bool BookAtomText::is_screen_renderable() { return true; }
bool BookAtomText::is_render_decomposable() { return true; };
Vector<wchar_t> BookAtomText::decompose() {
  Vector<wchar_t> data(str.begin(), str.end());
  return data;
};


  List<BYTE> * BookAtomText::serialize_binary()  {
    WORD size = 0;
    List<BYTE> *bytes = new List<BYTE>;
    for (auto c : str) {
      BYTE b0 = GetByteN(c, 0);
      BYTE b1 = GetByteN(c, 1);
      BYTE b2 = GetByteN(c, 2);
      bytes->push_back(b0);
      bytes->push_back(b1);
      bytes->push_back(b2);
    }
    DWORD len = bytes->size() + 1 /*Type size*/ + 4 /*my own size*/;
    bytes->push_front(getAtomType());
    BYTE b3 = GetByteN(len, 3) ;
    BYTE b2 = GetByteN(len, 2);
    BYTE b1 = GetByteN(len, 1);
    BYTE b0 = GetByteN(len, 0);

    bytes->push_front(b3);
    bytes->push_front(b2);
    bytes->push_front(b1);
    bytes->push_front(b0);
    return bytes;
  }