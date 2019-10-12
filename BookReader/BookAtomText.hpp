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
  ClassName getType() override{ return ClassName_BookAtomText; }
  bool is_screen_renderable() override;
  bool is_render_decomposable() override;
  Vector<wchar_t> decompose() override;
  UTF8String getText() { return str; }
  BookTextDirection get_dir() {

    // rewrite this funtion in better way
    for (auto c : decompose()) {
      if (c > 128)
        return BookTextDirection_RTL;
    }
    return BookTextDirection_LTR;
  }
  bool is_punc() {
    UTF8String chars(L".،؟,.;:()[]/%!#&_ +=«»…");
    int i = 0;
    for (auto c : chars) {
      UTF8String s1({c}); // c + L"";
      if (s1 == str)
        return true;
    }
    return false;
  }

  BookAtom *clone() override {
    BookAtom *rtn = new BookAtomText(str);
    return rtn;
  }

  friend bool operator==(BookAtomText &a, BookAtomText &b) {
    return a.str == b.str;
  }
  friend bool operator!=(BookAtomText &a, BookAtomText &b) {
    return a.str != b.str;
  }
  friend bool operator<(BookAtomText &a, BookAtomText &b) { return a < b; }

  // List<BYTE> * serialize_binary() override;
  BookAtom *deserialize_from_bin(DWORD len, BYTE *buf) override { throw 1; }

  List<BYTE> *serialize_binary() override ;
  // virtual BookAtom *deserialize_from_bin(DWORD len, BYTE *buf) override {
  // throw 2; }
};

#endif