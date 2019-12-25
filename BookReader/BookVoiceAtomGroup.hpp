#ifndef _H_MSD_BOOKVOICEATOMGROUP_
#define _H_MSD_BOOKVOICEATOMGROUP_
#include "../decomposable/decomposable.hpp"
#include "../defs/typedefs.hpp"
#include "../serializable/serializable.hpp"
#include "BookAtomGroup.hpp"
#include "BookAtomText.hpp"
#include "BookAtomVoice.hpp"
#include "clearTypeClass/cleartype.hpp"

class BookVoiceAtomGroup : public BookAtomGroup<BookAtomVoiceBase> {

public:
  BookVoiceAtomGroup(Vector<BookAtomVoiceBase *> voices)
      : BookAtomGroup<BookAtomVoiceBase>(voices) {}
  ClassName getType() override;
  DWORD getDuration() {
    DWORD rtn = 0;
    for (auto a : decompose()) {
      rtn += a->getDuration();
    }
    return rtn;
  }
};

ClassName BookVoiceAtomGroup::getType() {
  return ClassName::ClassName_BookVoiceAtomGroup;
};

#endif