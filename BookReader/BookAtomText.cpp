#include "BookAtomText.hpp"

BookAtomText::BookAtomText(UTF8String str)
    : str(str), BookAtom(BookAtomType_Text){};

bool BookAtomText::is_screen_renderable() { return true; }
bool BookAtomText::is_render_decomposable() { return true; };
Vector<wchar_t> BookAtomText::decompose() {
  Vector<wchar_t> data(str.begin(), str.end());
  return data;
};
