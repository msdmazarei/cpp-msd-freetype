#ifndef _H_MSD_BOOKATOMVoice_
#define _H_MSD_BOOKATOMVoice_
#include "BookAtom.hpp"
#include "clearTypeClass/cleartype.hpp"
#include "decomposable/decomposable.hpp"
#include "defs/typedefs.hpp"
enum VoiceAtomBinFormat {
  VoiceAtomBinFormat_MP4 = 0,
  VoiceAtomBinFormat_OGG = 1,
  VoiceAtomBinFormat_MP3 = 2
};

class BookAtomVoice : public BookAtomVoiceBase, public Decomposable<wchar_t> {
protected:
  VoiceAtomBinFormat format;
  DWORD bytesLength;
  BYTE *buffer;

public:
  BookAtomVoice(DWORD duration, DWORD bytesLength, VoiceAtomBinFormat format,
                BYTE *buf)
      : BookAtomVoiceBase(duration), BookAtom(BookAtomType_Voice),
        bytesLength(bytesLength), buffer(buf), format(format){};

  ClassName getType() override { return ClassName_BookAtomVoice; }
  bool is_screen_renderable() override;
  bool is_render_decomposable() override;
  Vector<wchar_t> decompose() override;
  virtual BYTE *getBuffer() { return buffer; }
  virtual DWORD getBufferLength() { return bytesLength; }
  BookAtom *clone() override {
    BookAtom *rtn = new BookAtomVoice(duration, bytesLength, format, buffer);
    return rtn;
  }

  DWORD getDuration() override { return duration; }
  // List<BYTE> * serialize_binary() override;
  // BookAtom *deserialize_from_bin(DWORD len, BYTE *buf) override { throw 1; }

  // List<BYTE> *serialize_binary() override;
};

#endif