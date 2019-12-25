#ifndef _H_MSD_BOOK_
#define _H_MSD_BOOK_
#include "BookAtomGroup.hpp"
#include "BookDirectionGroup.hpp"
#include "defs/typedefs.hpp"
#include <tuple>

class BookContent {
private:
  Vector<BookAtomGroup<BookAtom> *> title;
  BookPosIndicator indicator;
  WORD parentIndex;

public:
  BookContent(Vector<BookAtomGroup<BookAtom> *> title,
              BookPosIndicator indicator, WORD parentIndex)
      : title(title), indicator(indicator), parentIndex(parentIndex) {}
  Vector<BookAtomGroup<BookAtom> *> getTitle() { return title; }
  BookPosIndicator getPos() { return indicator; }
  WORD getParentIndex() { return parentIndex; }
};

enum BookFunc {
  BookFunc_nextPageImage = 1,
  BookFunc_renderMsdFormatPageAtPoint = 2,
  BookFunc_nextAtom = 3,
  BookFunc_prevAtom = 4,
  BookFunc_nextGroup = 5,
  BookFunc_prevGroup = 6,
  BookFunc_renderAtom = 7,
  BookFunc_getGroupAtomByPointer = 8
};
enum BookError {
  BookError_BookIsNotRenderable = 1,
  BookError_BadPointer = 2,
  BookError_NextAtomNotExists = 3,
  BookError_PrevAtomNotExists = 4,
  BookError_NextGroupNotExists = 5,
  BookError_PrevGroupNotExists = 6,
  BookError_BadATOM = 7
};
typedef struct BookError_ {
  BookFunc fn;
  BookError error;
} BookError_t;

class   Book : public Serializable<Book>,
             Decomposable<BookAtomGroup<BookAtom> *>,
             ClearTypeClass {
protected:
  BookType booktype;
  Vector<BookAtomGroup<BookAtom> *> groups;
  BookPosIndicator lastAtom;
  BookPosIndicator firstAtom;
  Vector<BookContent> Contents;
  DWORD totalAtoms;

  List<BYTE> *serialize_group(Vector<BookAtom *> &atoms,
                              BookAtomGroup<BookAtom> *group);

public:
  ClassName getType() override { return ClassName_Book; }
  bool is_render_decomposable() override { return true; };
  Vector<BookAtomGroup<BookAtom> *> decompose() override { return groups; };
  Book(BookType book_type):booktype(book_type){}
  Book(BookType book_type, Vector<BookAtomGroup<BookAtom> *> grps,
       Vector<BookContent> contents)
      : booktype(book_type), groups(grps), Contents(contents) {
    firstAtom = nextAtom(BookPosIndicator());
    lastAtom = BookPosIndicator({0, 0});
    auto groups = decompose();
    int lastgroupindex = groups.size() - 1;
    for (; lastgroupindex > -1; lastgroupindex--) {
      auto atoms = groups[lastgroupindex]->decompose();
      if (atoms.size() == 0) {

        continue;
      } else {
        lastAtom =
            BookPosIndicator({(WORD)lastgroupindex, (WORD)atoms.size() - 1});
        break;
      }
    }
    // calculate total atoms
    totalAtoms = 0;
    for (auto g : groups) {
      totalAtoms += g->decompose().size();
    }
  };
  DWORD getTotalAtoms() { return totalAtoms; }
  DWORD progress(BookPosIndicator ind) {
    DWORD p = 0;
    auto groups = decompose();
    if (ind.size() == 2) {
      for (int i = 0; i < ind[0]; i++)
        p += groups[i]->decompose().size();
      p += ind[1];
    }
    return p;
  }
  bool is_last_atom(BookPosIndicator ind) {

    if (ind.size() == 2)
      return (ind[0] >= lastAtom[0] && ind[1] >= lastAtom[1]);
    else
      return false;
  }
  bool is_first_atom(BookPosIndicator ind) {
    if (ind.size() == 2)
      return ind[0] <= firstAtom[0] && ind[1] <= firstAtom[1];
    else
      return false;
  }
  signed char compare(BookPosIndicator a, BookPosIndicator b) {
    int group_a = -1, group_b = -1, atom_a = -1, atom_b = -1;
    if (a.size() > 0)
      group_a = a[0];
    if (b.size() > 0)
      group_b = b[0];
    if (a.size() > 1)
      atom_a = a[1];
    if (b.size() > 1)
      atom_b = b[1];
    if (group_a == group_b && atom_a == atom_b)
      return 0;
    if (group_a > group_b)
      return 1;
    if (group_a < group_b)
      return -1;
    if (atom_a > atom_b)
      return 1;
    if (atom_a < atom_b)
      return -1;
    return 0;
  }
  BookAtom *findAtom(BookPosIndicator from_, BookPosIndicator to_,
                     bool (*filterFunc)(BookAtom *a)) {
    if (from_.size() != 2)
      throw "from_ has bad size.";
    if (to_.size() != 2)
      throw "to_ has bas size.";
    auto currentPointer = from_;
    while (true) {
      auto compare_current_pointer = compare(currentPointer, to_);
      if (compare_current_pointer == 1)
        break;
      auto current_group_i = currentPointer[0];
      auto current_atom_i = currentPointer[1];
      auto current_group = groups[current_group_i];
      auto current_atom = current_group->decompose()[current_atom_i];
      if (filterFunc(current_atom))
        return current_atom;
      if (compare_current_pointer == 0)
        break; // we reach to the end
      currentPointer = nextAtom(currentPointer);
    }
    return NULL;
  }
  template <typename T>
  List<T> mapOnAtoms(BookPosIndicator from_, BookPosIndicator to_,
                     T (*mapFunc)(BookAtom *a)) {
    if (from_.size() != 2)
      throw "from_ has bad size.";
    if (to_.size() != 2)
      throw "to_ has bas size.";
    auto currentPointer = from_;
    List<T> rtn;
    auto groups = decompose();
    while (true) {
      auto compare_current_pointer = compare(currentPointer, to_);
      if (compare_current_pointer == 1)
        break;
      auto current_group_i = currentPointer[0];
      auto current_atom_i = currentPointer[1];
      auto current_group = groups[current_group_i];
      auto current_atom = current_group->decompose()[current_atom_i];
      rtn.push_back(mapFunc(current_atom));
      if (compare_current_pointer == 0)
        break; // we reach to the end
      currentPointer = nextAtom(currentPointer);
    }
    return rtn;
  }
  BookType getBookType() { return booktype; }
  bool is_book_renderable() {
    return booktype == BookType_MSDFORMAT || booktype == BookType_PDF;
  }

  WORD nextGroup(Vector<WORD> pointer) {
    WORD pointer_size = pointer.size();
    WORD groups_size = decompose().size();
    int group_ptr = -1;
    if (pointer_size == 0) {
      if (groups_size == 0)
        throw BookError_t{BookFunc_nextGroup, BookError_NextGroupNotExists};
      // group_ptr++;
    } else {
      group_ptr = pointer[0];
    }
    group_ptr++;
    if (groups_size > group_ptr)
      return group_ptr;
    throw BookError_t{BookFunc_nextGroup, BookError_NextGroupNotExists};
  }
  WORD prevGroup(Vector<WORD> pointer) {
    WORD pointer_size = pointer.size();
    int group_ptr = -1;
    if (pointer_size == 0)
      throw BookError_t{BookFunc_prevGroup, BookError_PrevGroupNotExists};
    group_ptr = pointer[0];
    group_ptr--;
    if (group_ptr > -1)
      return group_ptr;

    throw BookError_t{BookFunc_prevGroup, BookError_PrevGroupNotExists};
  }
  BookAtom *getAtom(BookPosIndicator ind) {
    auto group_i = ind[0];
    auto atom_i = ind[1];
    auto groups = decompose();
    auto group = groups[group_i];
    auto atoms = group->decompose();
    auto atom = atoms[atom_i];
    return atom;
  }
  BookPosIndicator getFirstAtom() { return firstAtom; }
  BookPosIndicator getLastAtom() { return lastAtom; }
  BookPosIndicator nextAtom(BookPosIndicator pointer) {
    MLOG(" - next ATOM");
    int group_pointer = -1;
    int atom_pointer = -1;
    bool continue_loop = true;

    if (pointer.size() == 0) {
      try {
        group_pointer = nextGroup(pointer);
      } catch (BookError_t e) {
        MLOG(" - raise Exception ");
        if (e.error == BookError_NextGroupNotExists)
          throw BookError_t{BookFunc_nextAtom, BookError_NextAtomNotExists};
        else
          throw e;
      }
    } else {
      group_pointer = pointer[0];
    }

    if (pointer.size() > 1) {
      atom_pointer = pointer[1];
    }

    while (true) {
      atom_pointer++;
      auto g = decompose()[group_pointer];
      auto gclass = g->getType();
      auto atms = g->decompose();
      WORD atoms_count = atms.size();

      if (atoms_count > atom_pointer) {
        break;
      } else {
        try {
          pointer = BookPosIndicator{(WORD)group_pointer, (WORD)atom_pointer};
          group_pointer = nextGroup(pointer);
        } catch (BookError_t e) {
          if (e.error == BookError_NextGroupNotExists)
            throw BookError_t{BookFunc_nextAtom, BookError_NextAtomNotExists};
          else
            throw e;
        }
        atom_pointer = -1;
      }
    }
    if (group_pointer < 0 || atom_pointer < 0)
      throw "Invalid Group pointer, Atom pointer";
    MLOG("- Successfully goto next atom");
    return Vector<WORD>({(WORD)group_pointer, (WORD)atom_pointer});
  }
  Vector<WORD> prevAtom(Vector<WORD> pointer) {
    if (pointer.size() == 0)
      throw BookError_t{BookFunc_prevAtom, BookError_PrevAtomNotExists};
    int group_ind = pointer[0];
    int atom_ind = 0;
    if (pointer.size() > 1)
      atom_ind = pointer[1];

    atom_ind--;
    while (atom_ind < 0) {
      try {
        group_ind = prevGroup(Vector<WORD>({(WORD)group_ind}));
        atom_ind = decompose()[group_ind]->decompose().size() - 1;
      } catch (BookError_t e) {
        if (e.error == BookError_PrevGroupNotExists) {
          throw BookError_t{BookFunc_prevAtom, BookError_PrevAtomNotExists};
        } else
          throw e;
      }
    }
    if (group_ind < 0 || atom_ind < 0)
      throw "Invalid Group pointer, Atom pointer";
    return Vector<WORD>({(WORD)group_ind, (WORD)atom_ind});
  }

  std::tuple<BookAtomGroup<BookAtom> *, BookAtom *>
  getGroupAtomByPointer(Vector<WORD> pointer) {
    MLOG("getGroupAtomByPointer Called.")
    if (pointer.size() == 2) {
      WORD groupIndex = pointer[0];
      WORD wordIndex = pointer[1];
      BookAtomGroup<BookAtom> *group = decompose()[groupIndex];
      BookAtom *bookAtom = group->decompose()[wordIndex];
      MLOG("getGroupAtomByPointer - return group and bookAtom");
      return std::tuple<BookAtomGroup<BookAtom> *, BookAtom *>(group, bookAtom);

    } else {
      MLOG("getGroupAtomByPointer raise exception, cause pointer.size!=2");
      throw BookError_t{BookFunc_getGroupAtomByPointer, BookError_BadPointer};
    }
  }
  Vector<BookContent> getBookContent() { return Contents; }

  // List<BYTE> *serialize_binary() override;
  Book deserialize_from_bin(DWORD len, BYTE *buf) override;
  static Book *deserialize(DWORD index, BYTE *buf);
  static BookAtom *deserialize_atom(BYTE *buf, DWORD ind);
  static BookAtomGroup<BookAtom> *deserialize_group(BYTE *buf, DWORD ind,
                                                    Vector<BookAtom *> &atoms);
};

#endif