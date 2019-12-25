#ifndef _MEMORYBUFFER_
#define _MEMORYBUFFER_
#include "BookReader/MsdRandomReader.hpp"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

class MemoryBuffer {
protected:
  unsigned char *buf;
  unsigned long len;
  long pos;
  int f;
  MsdRandomReader *randomReader;

public:
  MemoryBuffer(MsdRandomReader *randomReader)
      : randomReader(randomReader) {
    len = randomReader->getByteLength();
    buf=NULL;
    pos = 0;

  }
  MemoryBuffer(unsigned char *buf, unsigned long len)
      : buf(buf), len(len), pos(0), randomReader(NULL) {
    buf = buf;
    len = len;
    pos = 0;
  }
  long getPos(){return pos;}

  ~MemoryBuffer() { MLOG("Memroy Buffer Deconstructor called."); }

  static ssize_t read_(void *fd, void *buf, size_t count) {
    // auto orig_count = count;
    MemoryBuffer *mbuf = (MemoryBuffer *)fd;

    if (mbuf->pos == mbuf->len) {

      return 0;
    }

    size_t avail_count = mbuf->len - mbuf->pos;
    count = count < avail_count ? count : avail_count;
    if (count == 0)
      return count;
    if (mbuf->buf != NULL) {
      unsigned char *bufaddr = mbuf->buf + mbuf->pos;
      memcpy(buf, bufaddr, count);
    } else {
      auto rtn = mbuf->randomReader->read(mbuf->pos,count);
      unsigned char *bufaddr = std::get<1>(rtn);
      count = std::get<0>(rtn);
      memcpy(buf, bufaddr, count);
      free(bufaddr);
    }

    mbuf->pos += count;
    return count;
  }
  static off_t lseek_(void *fd, off_t offset, int whence) {

    MemoryBuffer *mbuf = (MemoryBuffer *)fd;
    off_t new_offset;
    switch (whence) {
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset = mbuf->pos + offset;
      break;
    case SEEK_END:
      new_offset = mbuf->len + offset;
      break;

    default:
      break;
    }
    if (new_offset < 0)
      return EINVAL;
    if (new_offset > mbuf->len)
      return EOVERFLOW;
    mbuf->pos = new_offset;
    return mbuf->pos;
  }
  static void cleanup(void *fd) {
    MemoryBuffer *mbuf = (MemoryBuffer *)fd;
    delete mbuf;
  }
};
#endif