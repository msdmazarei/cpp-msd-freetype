#ifndef _MEMORYBUFFER_
#define _MEMORYBUFFER_
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

public:
  MemoryBuffer(unsigned char *buf, unsigned long len)
      : buf(buf), len(len), pos(0) {
    buf = buf;
    len = len;
    pos = 0;
  }
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
    unsigned char *bufaddr = mbuf->buf + mbuf->pos;
    memcpy(buf, bufaddr, count);
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