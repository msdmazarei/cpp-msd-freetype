#ifndef _H_MSD_LAZYBOOKATOMVoice_
#define _H_MSD_LAZYBOOKATOMVoice_
#include "BookAtomVoice.hpp"
#include "LazyBookAtom.hpp"
#include "LazyBookAtomBinary.hpp"
#include "MsdRandomReader.hpp"

class LazyBookAtomVoice :  public BookAtomVoiceBase, public LazyBookAtomBinary {
protected:
  VoiceAtomBinFormat format;
  DWORD atomLength;
  DWORD atomOffset;
  DWORD bufferOffset;
  MsdRandomReader *reader;

public:
  LazyBookAtomVoice(MsdRandomReader *reader, DWORD AtomBytePos,
                    DWORD atomLength, Vector<BYTE> encKey, DWORD duration,
                    VoiceAtomBinFormat format, DWORD bufferOffset,
                    DWORD encKeyIndex)
      : LazyBookAtomBinary(BookAtomType_Voice, reader, AtomBytePos, atomLength,
                           encKey, bufferOffset, encKeyIndex),
        BookAtomVoiceBase(duration), format(format),BookAtom(BookAtomType_Voice) {
    MLOG("HELLO FROM LazyBookAtomVoice Constructor");
  };

  ClassName getType() override { return ClassName_LazyBookAtomVoice; }

  DWORD getDuration() override { return duration; }
  // List<BYTE> * serialize_binary() override;
  // BookAtom *deserialize_from_bin(DWORD len, BYTE *buf) override { throw 1; }
};

#endif