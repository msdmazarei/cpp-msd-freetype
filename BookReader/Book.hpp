#ifndef _H_MSD_BOOK_
#define _H_MSD_BOOK_
#include "BookAtomGroup.hpp"
#include "BookDirectionGroup.hpp"
#include "defs/typedefs.hpp"
#include <tuple>

class BookContent {
private:
  Vector<uint16_t> Index;
  Vector<BookDirectionGroup<BookAtom> *> title;
  Vector<BookContent *> subContents;
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

class Book : public Serializable<Book>,
             Decomposable<BookAtomGroup<BookAtom> *>,
             ClearTypeClass {
protected:
  BookType booktype;
  Vector<BookAtomGroup<BookAtom> *> groups;
  BookContent Contents;

public:
  ClassName getType() { return ClassName_Book; }
  bool is_render_decomposable() { return true; };
  Vector<BookAtomGroup<BookAtom> *> decompose() { return groups; };
  Book(BookType book_type, Vector<BookAtomGroup<BookAtom> *> grps)
      : booktype(book_type), groups(grps){};
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
  Vector<WORD> nextAtom(Vector<WORD> pointer) {
    int group_pointer = -1;
    int atom_pointer = -1;
    bool continue_loop = true;

    if (pointer.size() == 0) {
      try {
        group_pointer = nextGroup(pointer);
      } catch (BookError_t e) {
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
    if (pointer.size() == 2) {
      WORD groupIndex = pointer[0];
      WORD wordIndex = pointer[1];
      BookAtomGroup<BookAtom> *group = decompose()[groupIndex];
      BookAtom *bookAtom = group->decompose()[wordIndex];
      return std::tuple<BookAtomGroup<BookAtom> *, BookAtom *>(group, bookAtom);

    } else {
      throw BookError_t{BookFunc_getGroupAtomByPointer, BookError_BadPointer};
    }
  }
};
#endif