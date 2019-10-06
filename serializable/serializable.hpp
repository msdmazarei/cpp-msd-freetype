#ifndef _H_MSD_SERIALIZER_
#define _H_MSD_SERIALIZER_
#include "defs/typedefs.hpp"

#include <istream>
template <typename T> class Serializable {
  virtual std::basic_istream<BYTE> serialize_binary() { throw 1; };
  virtual T deserialize_from_bin(std::basic_istream<BYTE>) { throw 2; };
};

#endif