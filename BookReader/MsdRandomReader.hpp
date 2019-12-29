#ifndef _MSD_RANDOM_READER_
#define _MSD_RANDOM_READER_

#include "typedefs.hpp"
#include "utils/jsreader.hpp"
#include <string.h>

class MsdRandomReader {
public:
  virtual DWORD getByteLength() = 0;
  virtual Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) = 0;
  ~MsdRandomReader(){};
};

typedef Tuple<DWORD, BYTE *>(readfn)(void *ctx, DWORD from, DWORD len);

class ProxyMsdRandomReader : public MsdRandomReader {
protected:
  readfn *fn;
  DWORD len;
  void *ctx;

public:
  ProxyMsdRandomReader(void *ctx, readfn *readfn, DWORD len)
      : ctx(ctx), fn(readfn), len(len) {}
  DWORD getByteLength() override { return len; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) override {
    return fn(ctx, from, len);
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

class JsMsdRandomReader : public MsdRandomReader {
protected:
  DWORD totalLen;
  DWORD id;

public:
  JsMsdRandomReader(DWORD id, DWORD len) : totalLen(len), id(id) {}
  DWORD getByteLength() override { return totalLen; }
  Tuple<DWORD, BYTE *> read(DWORD from, DWORD len) override {
    MLOG4("read function called, from:", from, " id: ", id);
    MLOG4("read function called, len:", len, "id:", id);
    if (totalLen >= (from + len)) {
      BYTE * buf = new BYTE[len];
      MLOG2("before call js_do_fetch id:", id);
      DWORD readDataLength = js_do_fetch(id, from, len,(void*)buf);
      MLOG2("after call js_do_fetch id:", id);
      // BYTE *b = (BYTE *)tmp;
      if ( readDataLength == 0) {
        MLOG2("js_do_fetch returned NULL", id);
        return Tuple<DWORD, BYTE *>(0, NULL);
      }
      // DWORD rtnlen = 0;
      // for (int i = 0; i < 4; i++) {
      //   rtnlen += (1 << (i * 8)) * b[i];
      // }
      MLOG4("js_do_fetch bytes len:", readDataLength, "id:", id);

      // BYTE *rtn = new BYTE[rtnlen];
      // memcpy(rtn, b + 4, rtnlen);
      MLOG2("free js_do_fetch buffer id:", id);
      // free(tmp);

      return Tuple<DWORD, BYTE *>(readDataLength, buf);
    } else {
      return Tuple<DWORD, BYTE *>(0, NULL);
    }
  }
};

#endif