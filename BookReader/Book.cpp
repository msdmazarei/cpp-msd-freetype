#include "Book.hpp"
#include <iostream>
Vector<BookAtom *> uniqAtoms(Vector<BookAtomGroup<BookAtom> *> &groups) {
  List<BookAtom *> uniqAtoms;
  for (auto g : groups) {
    for (auto a : g->decompose()) {

      // search in uAtoms for it
      bool existance = false;
      for (auto ap : uniqAtoms) {
        if (existance)
          break;
        if (ap->getAtomType() == a->getAtomType()) {
          auto aType = ap->getAtomType();
          switch (aType) {
          case BookAtomType_Text:
            existance = *((BookAtomText *)ap) == *((BookAtomText *)a);
            break;
          case BookAtomType_Control_NewLine:
          case BookAtomType_Control_NewPage:
            existance = true;
          default:
            break;
          }
        }
      }
      if (existance == false)
        uniqAtoms.push_back(a);
    }
  }

  Vector<BookAtom *> rtn(uniqAtoms.size());
  int i = 0;
  for (auto a : uniqAtoms) {
    rtn[i] = a;
    i++;
  }
  return rtn;
}

List<BYTE> *Book::serialize_group(Vector<BookAtom *> &atoms,
                                  BookAtomGroup<BookAtom> *group) {
  List<BYTE> *rtn = new List<BYTE>();
  for (auto ba : group->decompose()) {
    // try to find ba in atoms
    WORD atomIndex = 0;
    for (auto a : atoms) {
      bool found = false;
      if (a->getAtomType() == ba->getAtomType()) {
        switch (a->getAtomType()) {
        case BookAtomType_Text:

          found = (*((BookAtomText *)a) == *((BookAtomText *)ba));
          break;
        case BookAtomType_Control_NewLine:
        case BookAtomType_Control_NewPage:
          found = true;
        default:
          break;
        }
      }
      if (found)
        break;
      atomIndex++;
    }
    BYTE b0 = GetByteN(atomIndex, 0);
    BYTE b1 = GetByteN(atomIndex, 1);
    rtn->push_back(b0);
    rtn->push_back(b1);
  }
  if (group->getType() == ClassName_BookAtomGroup) {
    rtn->push_front(0);
  } else if (group->getType() == ClassName_BookDirectionGroup &&
             ((BookDirectionGroup<BookAtom> *)group)->getDirection() ==
                 BookTextDirection_LTR) {
    rtn->push_front(1);
  } else if (group->getType() == ClassName_BookDirectionGroup &&
             ((BookDirectionGroup<BookAtom> *)group)->getDirection() ==
                 BookTextDirection_RTL) {
    rtn->push_front(2);
  }
  DWORD Size = rtn->size() + 4 /*BYTE SIZE OF LEN*/;
  for (int i = 3; i > -1; i--) {
    BYTE b = GetByteN(Size, i);
    rtn->push_front(b);
  }
  return rtn;
}
List<BYTE> *Book::serialize_binary() {
  auto uAtoms = uniqAtoms(groups);
  List<BYTE> *atomTable = new List<BYTE>();
  for (auto a : uAtoms) {
    auto bytes = a->serialize_binary();
    for (auto b : *bytes)
      atomTable->push_back(b);
  }
  DWORD atomTableSize = atomTable->size() + 4 /*Size Bytes Len*/;
  for (int i = 3; i > -1; i--) {
    BYTE b = GetByteN(atomTableSize, i);
    atomTable->push_front(b);
  }

  List<BYTE> *BodyTable = new List<BYTE>();
  for (auto g : decompose()) {
    auto sb = serialize_group(uAtoms, g);
    for (BYTE b : *sb) {
      std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::endl;
    for (auto b : *sb)
      BodyTable->push_back(b);
  }

  // generate key
  auto key_len = 0; // rand() % 1024;
  Vector<BYTE> enc_key = Vector<BYTE>(key_len);
  for (int i = 0; i < key_len; i++)
    enc_key[i] = rand() % 256;

  List<BYTE> *rtn = new List<BYTE>();
  rtn->push_back(0);             // version
  rtn->push_back(getBookType()); // BookType

  for (int i = 0; i < 4; i++) {
    BYTE b = GetByteN(key_len + 4 /*its size*/, i);
    rtn->push_back(b);
  }
  for (int i = 0; i < key_len; i++)
    rtn->push_back(enc_key[i]); // copy enc key
  // copy atom table
  int i = 0;
  for (auto a : *atomTable) {
    if (key_len > 0)
      rtn->push_back(a ^ enc_key[i]);
    else
      rtn->push_back(a);
    if (key_len > 0)
      i = (i + 1) % (key_len);
  }
  // copy body table
  i = 0;
  for (auto a : *BodyTable) {
    if (key_len > 0)
      rtn->push_back(a ^ enc_key[i]);
    else
      rtn->push_back(a);
    if (key_len > 0)
      i = (i + 1) % (key_len);
  }
  return rtn;
};
Book Book::deserialize_from_bin(DWORD len, BYTE *buf) { throw 2; };
DWORD getItemSize(BYTE *buf, DWORD ind) {
  DWORD rtn = 0;
  for (int i = 3; i > -1; i--) {
    rtn = (rtn << 8);
    rtn += *(buf + ind + i);
  }
  return rtn;
}
BookAtom *Book::deserialize_atom(BYTE *buf, DWORD ind) {
  BookAtom *rtn = NULL;
  DWORD atomSize = getItemSize(buf, ind);
  ind += 4;
  BYTE atomType = *(buf + ind);
  ind++;
  switch (atomType) {
  case BookAtomType_Control_NewLine:
    rtn = new BookAtom(BookAtomType_Control_NewLine);
    break;
  case BookAtomType_Control_NewPage:
    rtn = new BookAtom(BookAtomType_Control_NewPage);
    break;

  case BookAtomType_Text: {

    DWORD textSize = (atomSize - 4 - 1) / 3;
    UTF8String txt;
    for (int i = 0; i < textSize; i++) {
      wchar_t c = 0;
      c =( c << 8) + *(buf + ind+2);
      c = (c << 8) + *(buf + ind+1);
      c = (c << 8) + *(buf + ind);
      ind+=3;
      txt.push_back(c);
    }
    rtn = new BookAtomText(txt);
  }; break;

  default:
    break;
  }
  return rtn;
}

Book *Book::deserialize(DWORD index, BYTE *buf) {
  DWORD ind = 0;
  BYTE file_version = *buf;
  ind++;
  if (file_version == 0) {
    BYTE booktype = *(buf + ind);
    ind++;
    DWORD key_len = getItemSize(buf, ind);
    ind += 4;
    Vector<BYTE> enc_key(key_len - 4);
    for (int i = 0; i < key_len - 4; i++) {
      enc_key[i] = *(buf + ind);
      ind++;
    }
    DWORD atomTableLen = 0;
    for (int i = 3; i > -1; i--) {
      if (key_len - 4 == 0)
        atomTableLen = (atomTableLen << 8) + (*(buf + ind + i));
      else
        atomTableLen = (atomTableLen << 8) + (*(buf + ind + i) ^ enc_key[i]);
    }
    ind += 4;
    // totaly decrypt atom table
    if (key_len - 4 != 0)
      for (int i = 0; i < atomTableLen; i++)
        *(buf + ind + i) = *(buf + ind + i) ^ enc_key[i];

    List<BookAtom *> bookAtoms;
    DWORD parsedAtoms = 0;
    while (parsedAtoms < atomTableLen - 4) {
      auto atomSize = getItemSize(buf, ind);
      auto atom = Book::deserialize_atom(buf, ind);
      parsedAtoms += atomSize;
      ind += atomSize;
      bookAtoms.push_back(atom);
    }
  }
  return NULL;
}
