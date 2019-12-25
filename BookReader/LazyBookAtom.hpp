#ifndef _MSD_H_LAZYBOOKATOM_
#define _MSD_H_LAZYBOOKATOM_

#include "BookAtom.hpp"
#include "MsdRandomReader.hpp"

class LazyBookAtom :virtual public  BookAtom {
protected:
  BookAtomType atom_type;
  MsdRandomReader *reader;
  DWORD atomBytePosition;
  DWORD atomLength;

public:
  LazyBookAtom(BookAtomType atom_type, MsdRandomReader *reader,
               DWORD AtomBytePos, DWORD atomLength)
      : reader(reader), atomBytePosition(AtomBytePos), atomLength(atomLength),
        BookAtom(atom_type){
          
        };
  ClassName getType() override { return ClassName_LazyBookAtom; };
  // virtual bool is_screen_renderable() { throw 1; };
  // virtual BookAtom *clone() {
  //   LazyBookAtom *rtn =
  //       new LazyBookAtom(atom_type, reader, atomBytePosition, atomLength);
  //   return rtn;
  // }
  // List<BYTE> *serialize_binary() override { throw "Not Implemented"; };
};

#endif