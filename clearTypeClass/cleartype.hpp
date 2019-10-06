#ifndef _H_MSD_CLEARTYPE_
#define _H_MSD_CLEARTYPE_
enum ClassName {
  ClassName_BookAtom = 0,
  ClassName_BookAtomGroup = 1,
  ClassName_BookAtomText = 2,
  ClassName_BookDirectionGroup = 3,
  ClassName_BookItem = 4,
  ClassName_Book = 5
};
class ClearTypeClass {
public:
  virtual ClassName getType() {throw 1;};
};
#endif