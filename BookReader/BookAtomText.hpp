#ifndef _H_MSD_BOOKATOMTEXT_
#define _H_MSD_BOOKATOMTEXT_
#include "BookAtom.hpp"
#include "clearTypeClass/cleartype.hpp"
#include "decomposable/decomposable.hpp"
#include "defs/typedefs.hpp"
class BookAtomText : public BookAtom, public Decomposable<wchar_t> {
protected:
  UTF8String str;

public:
  BookAtomText(UTF8String s);
  ClassName getType() { return ClassName_BookAtomText; }
  bool is_screen_renderable() override;
  bool is_render_decomposable() override;
  Vector<wchar_t> decompose() override;
  UTF8String getText(){
    return str;
  }

  BookAtom *clone() {
    BookAtom *rtn = new BookAtomText(str);
    return rtn;
  }

  friend bool operator==(BookAtomText &a, BookAtomText &b) {
    return a.str == b.str;
  }
  friend bool operator!=(BookAtomText &a, BookAtomText &b) {
    return a.str != b.str;
  }
};

#endif