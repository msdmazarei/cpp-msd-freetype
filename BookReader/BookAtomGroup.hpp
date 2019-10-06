#ifndef _H_MSD_BOOKATOMGROUP
#define _H_MSD_BOOKATOMGROUP
#include "BookAtom.hpp"
#include "clearTypeClass/cleartype.hpp"
#include "decomposable/decomposable.hpp"
#include "serializable/serializable.hpp"

template <typename BATOMT>
class BookAtomGroup : public Serializable<BookAtomGroup<BATOMT>>,
                      public Decomposable<BATOMT *>,
                      public ClearTypeClass {
protected:
  Vector<BATOMT *> atoms;

public:
  BookAtomGroup(Vector<BATOMT *> atoms);
  // ClassName getType() override;
  // bool is_render_decomposable() override;
  Vector<BATOMT *> decompose() override { return atoms; }
  ClassName getType() override { return ClassName_BookAtomGroup; }
  // std::basic_istream<BYTE> serialize_binary() override { throw 1; };
  // BookAtomGroup<BATOMT> deserialize_from_bin(std::basic_istream<BYTE>)
  // override {throw 2;};
};

template <class BATOMT>
BookAtomGroup<BATOMT>::BookAtomGroup(Vector<BATOMT *> atoms) : atoms(atoms) {}

#endif