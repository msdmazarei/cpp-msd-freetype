#ifndef _H_MSD_BOOKTEXTDIRECTIONGROUP_
#define _H_MSD_BOOKTEXTDIRECTIONGROUP_
#include "../decomposable/decomposable.hpp"
#include "../defs/typedefs.hpp"
#include "../serializable/serializable.hpp"
#include "BookAtomGroup.hpp"
#include "BookAtomText.hpp"
#include "clearTypeClass/cleartype.hpp"
template <typename BATOM>
class BookDirectionGroup : public BookAtomGroup<BATOM> {
private:
  BookTextDirection direction;

public:
  BookDirectionGroup(BookTextDirection direction, Vector<BATOM *> texts)
      : direction(direction), BookAtomGroup<BATOM>(texts) {}

  ClassName getType() override;
  BookTextDirection getDirection(){
    return direction;
  }
};

template <typename BATOM>

ClassName BookDirectionGroup<BATOM>::getType() {
  return ClassName::ClassName_BookDirectionGroup;
};

#endif