#ifndef _H_MSD_DECOMPOSABLE_
#define _H_MSD_DECOMPOSABLE_
#include "defs/typedefs.hpp"

template <typename ATOMT> class Decomposable {
public:
  virtual bool is_render_decomposable() {throw 1;} ;
  virtual Vector<ATOMT> decompose() {throw 2;};
};

#endif