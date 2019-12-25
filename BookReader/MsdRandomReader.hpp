#ifndef _MSDSTREAMREADER_
#define _MSDSTREAMREADER_

#include "typedefs.hpp"
#include <string.h>
typedef Tuple<DWORD, BYTE *> (readfn)(void *ctx,DWORD from, DWORD len) ;

class MsdRandomReader {
public:
  virtual DWORD getByteLength() = 0;
  virtual Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) = 0;
};
class ProxyMsdRandomReader : public MsdRandomReader {
protected:
  readfn *fn;
  DWORD len;
  void *ctx;
public:
  ProxyMsdRandomReader(void *ctx,readfn *readfn, DWORD len) :ctx(ctx), fn(readfn), len(len) {}
  DWORD getByteLength() override { return len; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) override {
    return fn(ctx,from, len);
  }
};

class MemroyMsdRandomReader : public MsdRandomReader {
protected:
  BYTE *buf;
  DWORD totalLen;

public:
  MemroyMsdRandomReader(BYTE *buf, DWORD len) : totalLen(len), buf(buf) {}
  DWORD getByteLength() override { return totalLen; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) override {
    if (totalLen > (from + len)) {
      BYTE *rtn = new BYTE[len];
      memcpy(rtn, buf + from, len);
      return Tuple<DWORD, BYTE *>(len, rtn);
    } else {
      return Tuple<DWORD, BYTE *>(0, NULL);
    }
  }
};

class FileMsdRandomReader : public MsdRandomReader {
protected:
  FILE *file;
  DWORD totalLen;

public:
  FileMsdRandomReader(const char *filename) {
    file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    totalLen = ftell(file);
  }
  ~FileMsdRandomReader() {
    if (file)
      fclose(file);
  }
  DWORD getByteLength() override { return totalLen; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) override {

    if (totalLen >= (from + len)) {
      BYTE *rtn = new BYTE[len];
      fseek(file, from, 0);
      fread(rtn, len, 1, file);
      return Tuple<DWORD, BYTE *>(len, rtn);
    } else {
      return Tuple<DWORD, BYTE *>(0, NULL);
    }
  }
};


class JsMsdRandomReader: public MsdRandomReader {

};

#endif