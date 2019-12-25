#ifndef _H_MSD_CLEARTYPE_
#define _H_MSD_CLEARTYPE_
enum ClassName {
  ClassName_BookAtom = 0,
  ClassName_BookAtomGroup = 1,
  ClassName_BookAtomText = 2,
  ClassName_BookDirectionGroup = 3,
  ClassName_BookItem = 4,
  ClassName_Book = 5,
  ClassName_BookAtomVoice = 6,
  ClassName_BookVoiceAtomGroup = 7,
  ClassName_BookAtomBinary = 8,
  ClassName_BookAtomPDF = 9,
  ClassName_BookAtomEPUB = 10,
  ClassName_BookAtomXPS = 11,

  ClassName_MsdLazyBook = 20 + 5,
  ClassName_LazyBookAtom = 20 + 0,
  ClassName_LazyBookAtomBinary = 8 + 20,
  ClassName_LazyBookAtomPDF = 9 + 20,
  ClassName_LazyBookAtomEPUB = 10 + 20,
  ClassName_LazyBookAtomXPS = 11 + 20,
  ClassName_LazyBookAtomVoice = 6+20,
  ClassName_LazyBookVoiceAtomGroup = 7+20,

};
class ClearTypeClass {
public:
  virtual ClassName getType() { throw 1; };
};
#endif