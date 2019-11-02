#ifndef MSD_MPGWRAPPER
#define MSD_MPGWRAPPER
#include <mpg123.h>
#include <string.h>
#include <defs/typedefs.hpp>

class MpgWrapper {

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

public:
  MpgWrapper(BYTE *buf, ULONG len) : buf(buf), buflen(len) {

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

    instance_count++;
    mpg123_open_feed(mh);
    mpg123_feed(mh, (const unsigned char *)buf, len);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    min_buffer_size_to_read = mpg123_outblock(mh);

    pcm_buf_len = min_buffer_size_to_read * mpg123_framelength(mh);
    pcm_buf = new BYTE[pcm_buf_len];
    size_t done = 0, total_done = 0;
    int last_read_res = 0;
    do {
      if (last_read_res < 0)
        break; // to ensure of exiting loop
      done = 0;
      mpg123_read(mh, pcm_buf + total_done, min_buffer_size_to_read, &done);
      total_done += done;
    } while (done > 0);
    if (total_done == 0) {
      free(pcm_buf);
    } else {
      pcm_buf_len = total_done;
    }
    std::cout << mpg123_framepos(mh) << std::endl;
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
      std::cout << "from>total duration " << std::endl;
      return std::tuple<BYTE *, size_t>(NULL, 0);
    }
    std::cout << "from_mili: " << from_second_milli
              << "to_milli: " << to_second_milli << "total_du: " << total_du
              << std::endl;
    auto chans = getChannels();
    auto sampleRate = getRate();
    auto coeff = chans * sizeof(INT16);
    unsigned long byteOffset_start = ((float)from_second_milli / (float)1000 *
                                      sizeof(INT16) * chans * sampleRate);
    if (byteOffset_start % coeff != 0)
      byteOffset_start -= byteOffset_start % coeff;

    unsigned long byteOffset_end = ((float)to_second_milli / (float)1000 *
                                    sizeof(INT16) * chans * sampleRate);
    if (byteOffset_end % coeff != 0)
      byteOffset_end -= byteOffset_end % coeff;

    std::cout << "all thing successfully done: "
              << "OFF_START: " << byteOffset_start
              << "OFF_END: " << byteOffset_end << "TOTAL SIZE: " << pcm_buf_len
              << std::endl;
    return std::tuple<BYTE *, size_t>((BYTE *)(pcm_buf + byteOffset_start),
                                      byteOffset_end - byteOffset_start);
  }

  ~MpgWrapper() {
    instance_count--;
    if (mh != NULL) {
      mpg123_delete(mh);
    }
    if (is_lib_inited == true && instance_count == 0) {
      mpg123_exit();
      is_lib_inited = false;
    }
  }
};

bool MpgWrapper::is_lib_inited = false;
ULONG MpgWrapper::instance_count = 0;
#endif