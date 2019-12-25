#ifndef _MSD_ATOM2RANDOMREADER_
#define _MSD_ATOM2RANDOMREADER_

#include "BookReader/LazyBookAtomBinary.hpp"
#include "defs/typedefs.hpp"

static Tuple<DWORD, BYTE *> readctx(void *ctx, DWORD f, DWORD l) {
  LazyBookAtomBinary *lctx =(LazyBookAtomBinary*)ctx;
  return lctx->read(f, l);
};

static MsdRandomReader *LazyBinAtomToRandomReader(LazyBookAtomBinary *atm) {
 return new ProxyMsdRandomReader(atm, &readctx, atm->getBufferLength());
}

#endif