#ifndef _MSD_H_BOOKATOM_
#define _MSD_H_BOOKATOM_
#include "clearTypeClass/cleartype.hpp"
#include "defs/typedefs.hpp"
#include "serializable/serializable.hpp"
#include <ios>

class BookAtom : public Serializable<BookAtom *>, public ClearTypeClass {
protected:
  BookAtomType atom_type;

public:
  BookAtom(BookAtomType type);
  virtual BookAtomType getAtomType() { return atom_type; }
  ClassName getType() override;
  virtual bool is_screen_renderable() { throw 1; };
  virtual BookAtom *clone() {
    BookAtom *rtn = new BookAtom(atom_type);
    return rtn;
  }
  List<BYTE> *serialize_binary() override;

  friend bool operator<(const BookAtom &a, const BookAtom &b) {
    return a.atom_type < b.atom_type;
  }
};

#endif