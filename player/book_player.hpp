#ifndef MSDBOOK_PLAYER_HPP
#define MSDBOOK_PLAYER_HPP
#include "BookReader/Book.hpp"
#include "BookReader/BookAtomVoice.hpp"
#include "BookReader/LazyBookAtomVoice.hpp"
#include "mpgwrapper2.hpp"
#include "streamers/atom2randomreader.hpp"
class BookPlayer {
protected:
  Book *book;

public:
  BookPlayer(Book *book) : book(book) {}
  ULONG getDurationFromTo(BookPosIndicator from_, BookPosIndicator to_) {
    auto mapFunc = this->getAtomDuration;

    auto durationList = book->mapOnAtoms<VOICEDURATION>(from_, to_, mapFunc);
    ULONG rtn = 0;
    for (auto a : durationList)
      rtn += a;
    return rtn;
  }
  static VOICEDURATION getAtomDuration(BookAtom *atom) {
    if (atom->getAtomType() == BookAtomType_Voice) {
      BookAtomVoiceBase *voice = dynamic_cast<BookAtomVoiceBase *>(atom);
      return voice->getDuration();
    } else {
      return 0;
    }
  }
  MpgWrapper2 *getVoiceAtomWrapper(BookPosIndicator ind) {
    BookAtom* atom = book->getAtom(ind);
    if (atom->getAtomType() == BookAtomType_Voice) {
      if (atom->getType() == ClassName_BookAtomVoice) {
        BookAtomVoice *vatom = dynamic_cast<BookAtomVoice *>(atom);
        return new MpgWrapper2(vatom->getBuffer(), vatom->getBufferLength());
      }
      if (atom->getType() == ClassName_LazyBookAtomVoice) {
        auto lazyBookAtomVoice = dynamic_cast<LazyBookAtomVoice*>(atom);
        auto s = lazyBookAtomVoice->getBufferLength();
        MLOG(lazyBookAtomVoice->getBufferLength());
        LazyBookAtomBinary *lbinatom = dynamic_cast<LazyBookAtomBinary*>(lazyBookAtomVoice);
        MsdRandomReader *mrr = LazyBinAtomToRandomReader(lbinatom);
            // new ProxyMsdRandomReader(lazyBookAtomVoice, &readctx, lazyBookAtomVoice->getBufferLength());
        auto mem = new MemoryBuffer(mrr);

        return new MpgWrapper2(mem);
      }
      return NULL;
    } else {
      return NULL;
    }
  }
};

#endif