#ifndef MSDBOOK_PLAYER_HPP
#define MSDBOOK_PLAYER_HPP
#include "Book.hpp"
#include "BookAtomVoice.hpp"
#include "mpgwrapper.hpp"

class BookPlayer {
protected:
  Book *book;

public:
  BookPlayer(Book *book) : book(book) {}
  ULONG getDurationFromTo(BookPosIndicator from_, BookPosIndicator to_) {
    auto durationList =
        book->mapOnAtoms<VOICEDURATION>(from_, to_, this->getAtomDuration);
    ULONG rtn = 0;
    for (auto a : durationList)
      rtn += a;
    return rtn;
  }
  VOICEDURATION getAtomDuration(BookAtom *atom) {
    if (atom->getAtomType() == BookAtomType_Voice) {
      BookAtomVoice *voice = (BookAtomVoice *)atom;
      return voice->getDuration();
    } else {
      return 0;
    }
  }
  MpgWrapper *getVoiceAtomWrapper(BookPosIndicator ind) {
    auto atom = book->getAtom(ind);
    if (atom->getAtomType() == BookAtomType_Voice) {
      BookAtomVoice *vatom = (BookAtomVoice *) atom;
      return new MpgWrapper(vatom->getBuffer(), vatom->getBufferLength());
    } else {
      return NULL;
    }
  }
};

#endif