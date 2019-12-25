#include "BookReader/MsdRandomReader.hpp"
#include "defs/typedefs.hpp"
#include "player/MemoryBuffer.hpp"
#include "streamers/fzStreamer.hpp"
#include <fstream>
#include <iostream>
#include <mupdf/fitz.h>
#include <stdio.h>
using namespace std;

static int msd_next_buffer(fz_context *ctx, fz_stream *stm, size_t max) {
  MemoryBuffer *state = (MemoryBuffer *)stm->state;
  auto bufferSize = 4096;
  BYTE *rtn = new BYTE[bufferSize];
  auto n = state->read_(state, rtn, bufferSize);
  /* n is only a hint, that we can safely ignore */
  // n = fread(state->buffer, 1, sizeof(state->buffer), state->file);
  // if (n < sizeof(state->buffer) && ferror(state->file))
  // 	fz_throw(ctx, FZ_ERROR_GENERIC, "read error: %s", strerror(errno));
  stm->rp = rtn;
  stm->wp = rtn + n;
  stm->pos += (int64_t)n;

  if (n == 0)
    return EOF;
  return *stm->rp++;
}

static void msd_seek_file(fz_context *ctx, fz_stream *stm, int64_t offset,
                          int whence) {
  MemoryBuffer *state = (MemoryBuffer *)stm->state;
  state->lseek_(state, offset, whence);
  stm->pos = state->getPos();
  stm->rp = stm->wp;
}
static void msd_drop_file(fz_context *ctx, void *state_) {
  MemoryBuffer *state = (MemoryBuffer *)state_;
  // delete state->
  // int n = fclose(state->file);
  // if (n < 0)
  // fz_warn(ctx, "close error: %s", strerror(errno));
  fz_free(ctx, state);
}

int main() {
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix ctm;

  ifstream ifs("/home/msd/Documents/Intel/1.pdf", ios::binary | ios::ate);
  ifstream::pos_type pos = ifs.tellg();

  // What happens if the OS supports really big files.
  // It may be larger than 32 bits?
  // This will silently truncate the value/
  int length = pos;

  // Manuall memory management.
  // Not a good idea use a container/.
  BYTE *pdfPtr = new BYTE[length];
  ifs.seekg(0, ios::beg);
  ifs.read((char *)pdfPtr, length);

  // No need to manually close.
  // When the stream goes out of scope it will close the file
  // automatically. Unless you are checking the close for errors
  // let the destructor do it.
  ifs.close();

  fz_context *ctx;

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  if (!ctx) {
    throw "cannot create mupdf context";
  }
  String throwmsg = "";
  /* Register the default file types to handle. */
  fz_try(ctx) fz_register_document_handlers(ctx);
  fz_catch(ctx) {
    fz_drop_context(ctx);
    throwmsg = "cannot register document handlers";
  }
  if (throwmsg != "")
    throw throwmsg;

  throwmsg = "";
  /* Open the document. */
  fz_stream *s ;
  fz_try(ctx) {
    BYTE *buffer = pdfPtr;
    //==============================================================
    FileMsdRandomReader *fmsdrandomreader =
        new FileMsdRandomReader("/home/msd/Documents/Intel/1.pdf");
    MemoryBuffer *mbuf = new MemoryBuffer(fmsdrandomreader);
    // fz_stream *msdStm =
    // fz_new_stream(ctx,mbuf,msd_next_buffer,msd_drop_file); msdStm->seek =
    // msd_seek_file;

    // fz_stream *s = msdStm;

    //==============================================================
    //
    s= FZStreamer::new_stream(ctx, mbuf);
    // fz_stream *s = fz_open_memory(ctx, buffer, length);
    // doc = fz_open_document(ctx, input);
    String input;
    switch (BookAtomType_PDF) {
    case BookAtomType_PDF:
      input = "f.pdf";
      break;
    case BookAtomType_EPUB:
      input = "f.epub";
      break;
    case BookAtomType_XPS:
      input = "f.xps";
      break;
    default:
      input = "UNKNWN";
      break;
    }
    MLOG2("fz_open_document_with_stream input:", input);
    doc = fz_open_document_with_stream(ctx, input.c_str(), s);
  }
  fz_catch(ctx) {
    // fprintf(stderr, "cannot open document: %s\n",
    // fz_caught_message(ctx));
    fz_drop_context(ctx);
    // return EXIT_FAILURE;
    throwmsg = "cannot open document";
  }
  if (throwmsg != "")
    throw throwmsg;

  throwmsg = "";
  int page_count;
  /* Count the number of pages. */
  fz_try(ctx) page_count = fz_count_pages(ctx, doc);
  fz_catch(ctx) {

    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    throwmsg = "cannot count number of pages";
  }
  if (throwmsg != "")
    throw throwmsg;
  int zoom = 100;
  int rotate = 0;
  int page_number = 50;
  int pageWidth = 500, pageHeight = 500;
  ctm = fz_scale(zoom / 100, zoom / 100);
  ctm = fz_pre_rotate(ctm, rotate);
  fz_layout_document(ctx, doc, pageWidth, pageHeight, 46);
  throwmsg = "";
  /* Render page to an RGB pixmap. */
  fz_try(ctx) pix = fz_new_pixmap_from_page_number(ctx, doc, page_number, ctm,
                                                   fz_device_rgb(ctx), 0);
  fz_catch(ctx) {
    // fprintf(stderr, "cannot render page: %s\n", fz_caught_message(ctx));
    throwmsg = "cannot render page";
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
  }
  if (throwmsg != "")
    throw throwmsg;
  throwmsg = "";
  fz_color_params cps;
  fz_buffer *buf = fz_new_buffer(ctx, 1000000);
  // fz_new_buffer_from_pixmap_as_png(ctx, pix, fz_default_color_params);
  fz_output *fzo = fz_new_output_with_buffer(ctx, buf);
  fz_write_pixmap_as_png(ctx, fzo, pix);
  const char *b = fz_string_from_buffer(ctx, buf);
  size_t a = fz_buffer_storage(ctx, buf, NULL);
  FILE *pFile;
  pFile = fopen("file.png", "wb");
  fwrite(b, a, 1, pFile);
  fclose(pFile);
  fz_drop_stream(ctx,s);
  fz_drop_document(ctx, doc);
  fz_drop_context(ctx);


  return 0;
}
