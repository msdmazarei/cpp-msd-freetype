#ifndef _H_MSD_BOOKATOMBinary_
#define _H_MSD_BOOKATOMBinary_
#include "BookAtom.hpp"
#include "clearTypeClass/cleartype.hpp"
#include "decomposable/decomposable.hpp"
#include "defs/typedefs.hpp"
enum BookAtomBinaryType {
  BookAtomBinaryTypes_PDF = 1,
  BookAtomBinaryTypes_EPUB = 2,
  BookAtomBinaryTypes_XPS = 3
};
class BookAtomBinary : public BookAtom, public Decomposable<BYTE> {
protected:
  WORD type;
  DWORD bytesLength;
  BYTE *buffer;

public:
  BookAtomBinary(DWORD bytesLength, BYTE *buf, WORD type)
      : BookAtom(BookAtomType_Binary), bytesLength(bytesLength), buffer(buf),
        type(type){};

  ClassName getType() override { return ClassName_BookAtomBinary; }
  bool is_screen_renderable() override{return true;};
  bool is_render_decomposable() override{return false;};
  Vector<BYTE> decompose() override { throw 1; };
  BYTE *getBuffer() { return buffer; }
  DWORD getBufferLength() { return bytesLength; }
  BookAtom *clone() override {
    BookAtom *rtn = new BookAtomBinary(bytesLength, buffer, type);
    return rtn;
  }
  BookAtom *deserialize_from_bin(DWORD len, BYTE *buf) override { throw 1; }

  List<BYTE> *serialize_binary() override{throw 1;};
};

class BookAtomPDF : public BookAtomBinary {
public:
  BookAtomPDF(DWORD bytesLength, BYTE *buf)
      : BookAtomBinary(bytesLength, buf, (WORD)BookAtomBinaryTypes_PDF) {
        atom_type=BookAtomType_PDF;
              }
  ClassName getType() override { return ClassName_BookAtomPDF; }
};
class BookAtomXPS : public BookAtomBinary {
public:
  BookAtomXPS(DWORD bytesLength, BYTE *buf)
      : BookAtomBinary(bytesLength, buf, (WORD)BookAtomBinaryTypes_XPS) {
                atom_type=BookAtomType_XPS;
      }
  ClassName getType() override { return ClassName_BookAtomXPS; }
};
class BookAtomEPUB : public BookAtomBinary {
public:
  BookAtomEPUB(DWORD bytesLength, BYTE *buf)
      : BookAtomBinary(bytesLength, buf, (WORD)BookAtomBinaryTypes_EPUB) {
        atom_type=BookAtomType_EPUB;
      }
  ClassName getType() override { return ClassName_BookAtomEPUB; }
};

#endif