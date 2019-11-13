#ifndef MSD_MPGWRAPPER2
#define MSD_MPGWRAPPER2
#include <mpg123.h>
#include <string.h>
#include <defs/typedefs.hpp>
#include "MemoryBuffer.hpp"

class MpgWrapper2 {

protected:
  BYTE *buf;
  ULONG buflen;
  mpg123_handle *mh = NULL;
  size_t done;
  int channels, encoding;
  long rate;
  size_t min_buffer_size_to_read;
  static bool is_lib_inited;
  static ULONG instance_count;
  BYTE *pcm_buf;
  ULONG pcm_buf_len;
  MemoryBuffer *mbuf;

public:
  MpgWrapper2(BYTE *buf, ULONG len) : buf(buf), buflen(len) {

    if (is_lib_inited == false) {
      if (mpg123_init() != MPG123_OK) {
        throw "MPG123 init faled";
      } else {
        is_lib_inited = true;
      }
    }

    int err;
    mpg123_handle *mpg_handler = mpg123_new(NULL, &err);
    if (err != MPG123_OK) {
      throw "Error to get mpg123 instance";
    }
    mh = mpg_handler;
    mbuf = new MemoryBuffer(buf, len);
    mpg123_replace_reader_handle(mh, MemoryBuffer::read_, MemoryBuffer::lseek_,
                                 MemoryBuffer::cleanup);
    mpg123_open_handle(mh, mbuf);


    instance_count++;
    auto scanres = mpg123_scan(mh);

    mpg123_getformat(mh, &rate, &channels, &encoding);
    min_buffer_size_to_read = mpg123_outblock(mh);
    auto lk =  mpg123_length(mh);
  }

  long getRate() { return rate; }
  int getChannels() { return channels; }
  int getEncodings() { return encoding; }
  long getlength() { return mpg123_length(mh); }
  float getTotalSecods() { return (float)getlength() / (float)getRate(); }

  std::tuple<BYTE *, size_t> get10Second(long from_second_milli) {

    long total_du = (long)(getTotalSecods() * 1000);
    long to_second_milli = from_second_milli + 10 * 1000;
    if (to_second_milli > total_du)
      to_second_milli = total_du;
    if (from_second_milli > total_du) {
      // std::cout << "from>total duration" << std::endl;
      return std::tuple<BYTE *, size_t>(NULL, 0);
    }
    auto rate = getRate();
    auto chans = getChannels();
    auto chanItemSize = sizeof(INT16);

    unsigned long bufferSize = (float)(to_second_milli - from_second_milli) /
                               (float)1000 * rate * chans * chanItemSize;
    if (bufferSize % (chans * chanItemSize) != 0) {
      auto remain = bufferSize % (chans * chanItemSize);
      bufferSize = bufferSize - remain;
    }
    BYTE *rtn = new BYTE[bufferSize];
    auto frametime = mpg123_timeframe(mh, from_second_milli / 1000);
    // std::cout << "pos:" << mpg123_tell(mh) << std::endl;
    auto seekValue = from_second_milli / 1000 * rate;
    auto seekres = mpg123_seek(mh, seekValue, 0);
    // std::cout << "seekres: " << seekres << std::endl;
    // auto seekframeres = mpg123_seek_frame(mh, frametime, 0);

    // std::cout << "seekframeres:" << seekframeres << std::endl;

    unsigned long readBuffer = 0;
    float readTime = 0;
    while ((bufferSize > readBuffer) &&
           readTime < (to_second_milli - from_second_milli)) {
      off_t n = 0;
      unsigned char *abuf;
      size_t si;
      auto readres = mpg123_decode_frame(mh, &n, &abuf, &si);
      if (readres != MPG123_OK)
        break;
      if (si + readBuffer > bufferSize)
        si = bufferSize - readBuffer;
      memcpy(rtn + readBuffer, abuf, si);
      readBuffer += si;
      readTime +=
          (float)si / (float)chans / (float)chanItemSize * 1000 / (float)rate;
    }
    return std::tuple<BYTE *, size_t>((BYTE *)rtn, readBuffer);
  }

  ~MpgWrapper2() {
    instance_count--;
    if (mh != NULL) {
      mpg123_delete(mh);
    }
    if (mbuf)
      delete mbuf;

    if (is_lib_inited == true && instance_count == 0) {
      mpg123_exit();
      is_lib_inited = false;
    }
  }
};

bool MpgWrapper2::is_lib_inited = false;
ULONG MpgWrapper2::instance_count = 0;

#endif