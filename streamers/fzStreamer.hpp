#ifndef _MSD_FZSTREAMER_
#define _MSD_FZSTREAMER_
#include "player/MemoryBuffer.hpp"
#include <mupdf/fitz.h>

class FZStreamer {
protected:
  const int bufferSize = 10 * 1024; // 10K
  BYTE *buffer;
  MemoryBuffer *mbuf;

public:
  FZStreamer(MemoryBuffer *mbuf) : mbuf(mbuf) { buffer = new BYTE[bufferSize]; }

  static int msd_next_buffer(fz_context *ctx, fz_stream *stm, size_t max) {
    FZStreamer *state = (FZStreamer *)stm->state;
    auto n = state->mbuf->read_(state->mbuf, state->buffer, state->bufferSize);
    /* n is only a hint, that we can safely ignore */
    // n = fread(state->buffer, 1, sizeof(state->buffer), state->file);
    // if (n < sizeof(state->buffer) && ferror(state->file))
    // 	fz_throw(ctx, FZ_ERROR_GENERIC, "read error: %s", strerror(errno));
    stm->rp = state->buffer;
    stm->wp = state->buffer + n;
    stm->pos += (int64_t)n;

    if (n == 0)
      return EOF;
    return *stm->rp++;
  }

  static void msd_seek_file(fz_context *ctx, fz_stream *stm, int64_t offset,
                            int whence) {
    FZStreamer *state = (FZStreamer *)stm->state;
    state->mbuf->lseek_(state->mbuf, offset, whence);
    stm->pos = state->mbuf->getPos();
    stm->rp = stm->wp;
  }
  static void msd_drop_file(fz_context *ctx, void *state_) {
    FZStreamer *state = (FZStreamer *)state_;
    // delete state->
    // int n = fclose(state->file);
    // if (n < 0)
    // fz_warn(ctx, "close error: %s", strerror(errno));
    free(state->buffer);
    delete (state->mbuf);

    fz_free(ctx, state);
    // delete state;
  }
  static fz_stream *new_stream(fz_context *ctx, MemoryBuffer *mbuf) {
      FZStreamer *msdstreamer = new FZStreamer(mbuf); 
    fz_stream *msdStm =
        fz_new_stream(ctx, msdstreamer, msd_next_buffer, msd_drop_file);
    msdStm->seek = msd_seek_file;
    return msdStm;
  }
};

#endif