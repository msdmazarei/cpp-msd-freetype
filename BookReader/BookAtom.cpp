#include "BookAtom.hpp"

BookAtom::BookAtom(BookAtomType atom_type) : atom_type(atom_type) {}
ClassName BookAtom::getType() { return ClassName_BookAtom; }

// List<BYTE> *BookAtom::serialize_binary() {
//   List<BYTE> *bytes = new List<BYTE>;
//   DWORD len = 1 /*Type size*/ + 4 /*my own size*/;
//   bytes->push_front(getAtomType());
//   BYTE b3 = GetByteN(len, 3);
//   BYTE b2 = GetByteN(len, 2);
//   BYTE b1 = GetByteN(len, 1);
//   BYTE b0 = GetByteN(len, 0);

//   bytes->push_front(b3);
//   bytes->push_front(b2);
//   bytes->push_front(b1);
//   bytes->push_front(b0);
//   return bytes;
// }
