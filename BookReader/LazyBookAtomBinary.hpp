#ifndef _H_MSD_LAZYBOOKATOMBinary_
#define _H_MSD_LAZYBOOKATOMBinary_

#include "LazyBookAtom.hpp"

class LazyBookAtomBinary : public LazyBookAtom {
protected:
  DWORD bufferOffset;
  Vector<BYTE> encKey;
  DWORD encKeySize;
  DWORD bufferLength;
  DWORD encKeyIndex;

public:
  LazyBookAtomBinary(BookAtomType atom_type, MsdRandomReader *reader,
                     DWORD AtomBytePos, DWORD atomLength, Vector<BYTE> encKey,
                     DWORD bufferOffset, DWORD encKeyIndex)
      : LazyBookAtom(atom_type, reader, AtomBytePos, atomLength),
        bufferOffset(bufferOffset), encKeyIndex(encKeyIndex),
        BookAtom(atom_type) {
    encKeySize = encKey.size();
    if (atomLength - bufferOffset > 0)
      bufferLength = atomLength - bufferOffset;
    else
      bufferLength = 0;
  };

  ClassName getType() override { return ClassName_LazyBookAtomBinary; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) {
    auto rtn = reader->read(from + bufferOffset+atomBytePosition, len);
    auto rtnLen = std::get<0>(rtn);
    auto buf = std::get<1>(rtn);
    if (rtnLen > 0 && encKeySize > 0) {
      BYTE *unencBuf = new BYTE[rtnLen];
      memcpy(unencBuf, buf, rtnLen);
      for (int i = 0; i < rtnLen; i++)
        unencBuf[i] =
            unencBuf[i] ^
            encKey[(i + from + bufferOffset + encKeyIndex) % encKeySize];
      return Tuple<DWORD, BYTE *>(rtnLen, unencBuf);
    } else
      return rtn;
  }
  DWORD getBufferLength() { return bufferLength; }
  // List<BYTE> *serialize_binary() override { throw 1; };
};

class LazyBookAtomPDF : public LazyBookAtomBinary {
public:
  LazyBookAtomPDF(MsdRandomReader *reader, DWORD AtomBytePos, DWORD atomLength,
                  Vector<BYTE> encKey, DWORD bufferOffset, DWORD encKeyIndex)
      : LazyBookAtomBinary(BookAtomType_PDF, reader, AtomBytePos, atomLength, encKey,
                           bufferOffset, encKeyIndex),
        BookAtom(BookAtomType_PDF) {
    // atom_type = BookAtomType_PDF;
  }
  ClassName getType() override { return ClassName_LazyBookAtomPDF; }
};
class LazyBookAtomXPS : public LazyBookAtomBinary {
public:
  LazyBookAtomXPS(MsdRandomReader *reader, DWORD AtomBytePos, DWORD atomLength,
                  Vector<BYTE> encKey, DWORD bufferOffset, DWORD encKeyIndex)
      : LazyBookAtomBinary(BookAtomType_XPS, reader, AtomBytePos, atomLength, encKey,
                           bufferOffset, encKeyIndex),
        BookAtom(BookAtomType_XPS) {
    atom_type = BookAtomType_XPS;
  }
  ClassName getType() override { return ClassName_LazyBookAtomXPS; }
};
class LazyBookAtomEPUB : public LazyBookAtomBinary {
public:
  LazyBookAtomEPUB(MsdRandomReader *reader, DWORD AtomBytePos, DWORD atomLength,
                   Vector<BYTE> encKey, DWORD bufferOffset, DWORD encKeyIndex)
      : LazyBookAtomBinary(BookAtomType_EPUB, reader, AtomBytePos, atomLength, encKey,
                           bufferOffset, encKeyIndex),
        BookAtom(BookAtomType_EPUB) {
    atom_type = BookAtomType_EPUB;
  }
  ClassName getType() override { return ClassName_LazyBookAtomEPUB; }
};

#endif