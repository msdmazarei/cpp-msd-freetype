#ifndef _MSD_BOOK_RENDERER
#define _MSD_BOOK_RENDERER

#include "BookReader/Book.hpp"
#include "BookReader/BookAtomBinary.hpp"
#include "BookReader/LazyBookAtomBinary.hpp"
#include "book_renderer_formats.hpp"
#include "defs/typedefs.hpp"
#include "renderer/msd_renderer.hpp"
#include "streamers/atom2randomreader.hpp"
#include "streamers/fzStreamer.hpp"
#include "utils/imageUtils.hpp"
#include <map>
#include <mupdf/fitz.h>
#include <tuple>

enum BookRendererFunc {
  BookRendererFunc_nextPageImage = 1,
  BookRendererFunc_renderMsdFormatPageAtPoint = 2,
  BookRendererFunc_nextAtom = 3,
  BookRendererFunc_prevAtom = 4,
  BookRendererFunc_nextGroup = 5,
  BookRendererFunc_prevGroup = 6,
  BookRendererFunc_renderAtom = 7
};
enum BookRendererError {
  BookRendererError_BookIsNotRenderable = 1,
  BookRendererError_BadPointer = 2,
  BookRendererError_NextAtomNotExists = 3,
  BookRendererError_PrevAtomNotExists = 4,
  BookRendererError_NextGroupNotExists = 5,
  BookRendererError_PrevGroupNotExists = 6,
  BookRendererError_BadATOM = 7
};
typedef struct BookRendererError_ {
  BookRendererFunc fn;
  BookRendererError error;
} BookRendererError_t;

class MsdBookRendererPage;
class VirtualMsdBookRendererPage;
class BookRenderer {
protected:
  Book *book;
  BookRendererFormat *rendererFormat;
  BookPosIndicator bookPointer;
  WORD pageWidth;
  WORD pageHeight;
  Map<BookAtom *, TextImage> atomImages;
  TextRenderer *txtRenderer;
  BookAtom *specAtom; // for pdf,epub books
  BYTE *current_font_buf;
  WORD font_buf_len;
  WORD page_count;

  // pdf
  fz_context *ctx;
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix ctm;

public:
  BookRenderer(Book *book, BookRendererFormat *rendererFormat, WORD width,
               WORD height)
      : book(book), rendererFormat(rendererFormat), pageWidth(width),
        pageHeight(height), txtRenderer(NULL) {

    if (book->getBookType() == BookType_PDF ||
        book->getBookType() == BookType_EPUB) {
      // find spec atom
      specAtom = book->findAtom(book->getFirstAtom(), book->getLastAtom(),
                                BookRenderer::is_pdf_xps_epub_atom);
      if (specAtom == NULL)
        throw "no pdf found in book";

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
      fz_stream *s;
      fz_try(ctx) {
        if (specAtom->getType() == ClassName_LazyBookAtomPDF) {
          LazyBookAtomPDF *latom = dynamic_cast<LazyBookAtomPDF *>(specAtom);
          MsdRandomReader *mrr = LazyBinAtomToRandomReader(latom);
          MemoryBuffer *mbuf = new MemoryBuffer(mrr);
          s = FZStreamer::new_stream(ctx, mbuf);
        } else {
          BookAtomBinary *bab = (BookAtomBinary *)specAtom;
          BYTE *buffer = bab->getBuffer();

          s = fz_open_memory(ctx, buffer, bab->getBufferLength());
        }

        // doc = fz_open_document(ctx, input);
        String input;
        MLOG2("file type to open: ", specAtom->getAtomType());
        switch (specAtom->getAtomType()) {
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
      /* Count the number of pages. */
      fz_try(ctx) page_count = fz_count_pages(ctx, doc);
      fz_catch(ctx) {

        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        throwmsg = "cannot count number of pages";
      }
      if (throwmsg != "")
        throw throwmsg;
    }
  }
  ~BookRenderer() {
    if (doc) {
      fz_drop_document(ctx, doc);
    }
    if (ctx) {
      fz_drop_context(ctx);
    }
  }
  static bool is_pdf_xps_epub_atom(BookAtom *a) {
    switch (a->getAtomType()) {
    case BookAtomType_EPUB:
    case BookAtomType_XPS:
    case BookAtomType_PDF:
      return true;
      break;

    default:
      return false;
      break;
    }
  }

  BookRendererFormat *getBookRendererFormat() { return rendererFormat; }
  void setBookRendererFormat(BookRendererFormat *newBookRendererFormat) {
    rendererFormat = newBookRendererFormat;
  }
  Book *getBook() { return book; }
  BookPosIndicator getBookPointer() { return bookPointer; }
  WORD getPageCount() { return page_count; }
  Vector<BookPosIndicator> getPageIndicators();
  Tuple<size_t, const char *> renderDocPage(BookPosIndicator pointer, WORD zoom,
                                            WORD rotate) {
    if (!(pointer.size() == 2 && pointer[0] == (WORD)-1 && pointer[1] < page_count)) {
      throw "BAD POINTER";
    }
    auto page_number = pointer[1];
    /* Compute a transformation matrix for the zoom and rotation desired. */
    /* The default resolution without scaling is 72 dpi. */
    ctm = fz_scale(zoom / 100, zoom / 100);
    ctm = fz_pre_rotate(ctm, rotate);
    fz_layout_document(ctx, doc, pageWidth, pageHeight,
                       rendererFormat->getFontSize());
    String throwmsg = "";
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
    return Tuple<size_t, const char *>(a, b);
  }
  MsdBookRendererPage
  //, BookPosIndicator>
  renderMsdFormatPageAtPointFW(Vector<WORD> pointer);
  MsdBookRendererPage
  //, BookPosIndicator>
  renderMsdFormatPageAtPointBack(Vector<WORD> pointer);
  RGBAImage nextPageImage();

  TextImage *renderAtom(BookAtomGroup<BookAtom> *bookatomGroup,
                        BookAtom *bookatom) {
    MLOG("RENDER ATOM CALLED");
    if (txtRenderer == NULL) {
      MLOG("creating text renderer");
      auto p = rendererFormat->getFontBuffer();
      long pl = (long)(void *)p;
      MLOG2("font buffer:", pl);
      MLOG2("buffer font Size:", rendererFormat->getFontBufferSize());
      txtRenderer = new TextRenderer(rendererFormat->getFontBuffer(),
                                     rendererFormat->getFontBufferSize());
    }
    MLOG("set font size");
    txtRenderer->set_font_size(rendererFormat->getFontSize());

    BookTextDirection txtDirection = BookTextDirection_LTR;
    if (bookatomGroup != NULL)
      if (bookatomGroup->getType() == ClassName_BookDirectionGroup)
        txtDirection =
            ((BookDirectionGroup<BookAtom> *)bookatomGroup)->getDirection();

    auto searchResult = atomImages.find(bookatom);
    if (searchResult != atomImages.end()) {
      // something is found. it is rendered before.
      return &(searchResult->second);
    }
    // std::cout << "type:" << bookatom->getAtomType();
    switch (bookatom->getAtomType()) {
    case BookAtomType_Text:
      if (true) { // to create new block.
        BookAtomText *txt = (BookAtomText *)bookatom;
        UTF8String text = txt->getText();
        // std::wcout << text << std::endl;

        auto foreC = rendererFormat->getTextForeColor();
        auto backC = rendererFormat->getTextBackColor();
        if (text == L" ") {
          RGBAImage imgSpace = RGBAImage(rendererFormat->getFontSize() / 5,
                                         rendererFormat->getFontSize() / 5);
          for (int x = 0; x < imgSpace.get_width(); x++)
            for (int y = 0; y < imgSpace.get_height(); y++)
              imgSpace.set_pixel(x, y, backC);
          auto r = new TextImage();
          r->base_line = imgSpace.get_height();
          r->image = imgSpace;
          return r;
        }
        auto vc = UTF8toByteBuffer(text);
        auto vc_size = vc.size();
        auto vc_data = vc.data();
        MLOG("rendering atom to bitmap");
        TextBitmap tmpTxtBmp =
            txtRenderer->render((char *)vc_data, vc_size, txtDirection);
        RGBAImage img =
            txtRenderer->colorizeBitmap(tmpTxtBmp.bitmap, foreC, backC);
        // img.write("atom.png");
        MLOG2("Text Image return with baseline", tmpTxtBmp.base_line);
        TextImage txtImage{img, tmpTxtBmp.base_line};
        atomImages.insert(std::make_pair(bookatom, txtImage));
        return new TextImage(txtImage);
        break;
      }
    default:
      // std::cout << "UNKNOWN TYPE" << std::endl;
      throw BookRendererError_t{BookRendererFunc_renderAtom,
                                BookRendererError_BadATOM};
      break;
    }
  }
};

class VirtualMsdBookRendererLine {
protected:
  Book *book;
  BookRendererFormat *rendererFormat;
  BookPosIndicator startLinePosInd;
  BookPosIndicator endLinePosInd;
  WORD pageWidth;

  Vector<BookAtom *> lineAtoms;
  Vector<BookAtomGroup<BookAtom> *> lineGroups;

  Vector<TextImage *> atomImages;
  Vector<BookPosIndicator> atomPosInd;

  RGBAImage *lineImagePtr;
  BookRenderer *bookRenderer;
  RenderDirection rDirection;
  int maxImageHeight;
  int maxBaseLine;

public:
  VirtualMsdBookRendererLine(Book *book, Vector<WORD> pointer, WORD pageWidth,
                             BookRenderer *bookRenderer,
                             RenderDirection rDirection)
      : book(book), startLinePosInd(pointer), pageWidth(pageWidth),
        bookRenderer(bookRenderer), rDirection(rDirection), lineImagePtr(NULL),
        maxImageHeight(0), maxBaseLine(0) {
    MLOG("MsdBookRendererLine Constructor Called.");
  }
  //   MsdBookRendererLine(Book *book) : book(book), rendererFunc(NULL){};
  //   MsdBookRendererLine(TextImage *(&rendererFunc)(
  //       BookAtomGroup<BookAtom> *bookatomGroup, BookAtom *bookatom))
  //       : rendererFunc(rendererFunc) {}

  Vector<BookPosIndicator> getAtomPointersForALine() {
    MLOG("getAtomPointersForALine called.");
    List<BookAtom *> l_lineAtoms;
    // List<BookAtomGroup<BookAtom> *> l_lineGroups;
    // List<TextImage *> l_atomImages;
    // List<BookPosIndicator> l_atomPosInd;

    BookPosIndicator tmpPos = startLinePosInd;
    WORD cW = 0;
    while (true) {
      MLOG("in loop getting group_and_atom");
      auto group_and_atom = book->getGroupAtomByPointer(tmpPos);
      MLOG(" group and atom returned as tuple");
      auto groupPtr = std::get<0>(group_and_atom);
      auto atomPtr = std::get<1>(group_and_atom);
      MLOG("group and atom extracted");
      if (atomPtr->getAtomType() == BookAtomType_Control_NewLine) {
        MLOG("NEW LINE ATOM");
        if (l_lineAtoms.size() == 0) {
          try {
            if (rDirection == RenderDirection_Forward)
              tmpPos = book->nextAtom(tmpPos);
            else
              tmpPos = book->prevAtom(tmpPos);
          } catch (const BookError_t &e) {
            if (e.error == BookError_NextAtomNotExists ||
                e.error == BookError_PrevAtomNotExists)
              break;
          }

          continue;
        } else
          break;
      }
      MLOG("RENDERING ATOM");
      auto txtImgPtr = bookRenderer->renderAtom(groupPtr, atomPtr);
      MLOG("ATOM RENDERERD");
      auto atomImageWidth = txtImgPtr->image.get_width();

      if (cW + atomImageWidth > pageWidth - 1)
        break;
      cW += atomImageWidth;

      maxImageHeight = MAXVAL(txtImgPtr->image.get_height(), maxImageHeight);
      maxBaseLine = MAXVAL(txtImgPtr->base_line, maxBaseLine);
      l_lineAtoms.push_back(atomPtr);
      // l_lineGroups.push_back(groupPtr);
      // l_atomImages.push_back(txtImgPtr);
      try {
        if (rDirection == RenderDirection_Forward)
          tmpPos = book->nextAtom(tmpPos);
        else
          tmpPos = book->prevAtom(tmpPos);
      } catch (const BookError_t &e) {
        if (e.error == BookError_NextAtomNotExists ||
            e.error == BookError_PrevAtomNotExists)
          break;
      }
    }
    endLinePosInd = tmpPos;

    // atomImages = Vector<TextImage *>(l_atomImages.begin(),
    // l_atomImages.end());
    return Vector<BookPosIndicator>();
  }
  WORD getMaxHeight() { return maxImageHeight; }
  WORD getMaxBaseLine() {
    return maxBaseLine;
    WORD max_baseline = 0;
    for (auto c : atomImages)
      max_baseline = MAXVAL(c->base_line, max_baseline);
    return max_baseline;
  }
  Vector<WORD> getEndLinePointer() { return endLinePosInd; }
  // Vector<BookAtom *> getBookAtoms() { return lineAtoms; }
  // Vector<BookAtomGroup<BookAtom> *> getBookAtomGroups() { return lineGroups;
  // }
};

class MsdBookRendererLine {
protected:
  Book *book;
  BookRendererFormat *rendererFormat;
  BookPosIndicator startLinePosInd;
  BookPosIndicator endLinePosInd;
  WORD pageWidth;

  Vector<BookAtom *> lineAtoms;
  Vector<BookAtomGroup<BookAtom> *> lineGroups;

  Vector<TextImage *> atomImages;
  Vector<BookPosIndicator> atomPosInd;

  RGBAImage *lineImagePtr;
  BookRenderer *bookRenderer;
  RenderDirection rDirection;

public:
  MsdBookRendererLine(Book *book, Vector<WORD> pointer, WORD pageWidth,
                      BookRenderer *bookRenderer, RenderDirection rDirection)
      : book(book), startLinePosInd(pointer), pageWidth(pageWidth),
        bookRenderer(bookRenderer), rDirection(rDirection), lineImagePtr(NULL) {
    MLOG("MsdBookRendererLine Constructor Called.");
  }
  //   MsdBookRendererLine(Book *book) : book(book), rendererFunc(NULL){};
  //   MsdBookRendererLine(TextImage *(&rendererFunc)(
  //       BookAtomGroup<BookAtom> *bookatomGroup, BookAtom *bookatom))
  //       : rendererFunc(rendererFunc) {}

  Vector<BookPosIndicator> getAtomPointersForALine() {
    MLOG("getAtomPointersForALine called.");
    List<BookPosIndicator> rtn;
    List<BookAtom *> l_lineAtoms;
    List<BookAtomGroup<BookAtom> *> l_lineGroups;
    List<TextImage *> l_atomImages;
    List<BookPosIndicator> l_atomPosInd;

    BookPosIndicator tmpPos = startLinePosInd;
    WORD cW = 0;
    while (true) {
      MLOG("in loop getting group_and_atom");
      auto group_and_atom = book->getGroupAtomByPointer(tmpPos);
      MLOG(" group and atom returned as tuple");
      auto groupPtr = std::get<0>(group_and_atom);
      auto atomPtr = std::get<1>(group_and_atom);
      MLOG("group and atom extracted");
      if (atomPtr->getAtomType() == BookAtomType_Control_NewLine) {
        MLOG("NEW LINE ATOM");
        if (l_lineAtoms.size() == 0) {
          try {
            if (rDirection == RenderDirection_Forward)
              tmpPos = book->nextAtom(tmpPos);
            else
              tmpPos = book->prevAtom(tmpPos);
          } catch (const BookError_t &e) {
            if (e.error == BookError_NextAtomNotExists ||
                e.error == BookError_PrevAtomNotExists)
              break;
          }

          continue;
        } else
          break;
      }
      MLOG("RENDERING ATOM");
      auto txtImgPtr = bookRenderer->renderAtom(groupPtr, atomPtr);
      MLOG("ATOM RENDERERD");
      auto atomImageWidth = txtImgPtr->image.get_width();
      if (cW + atomImageWidth > pageWidth - 1)
        break;
      cW += atomImageWidth;
      rtn.push_back(tmpPos);
      l_lineAtoms.push_back(atomPtr);
      l_lineGroups.push_back(groupPtr);
      l_atomImages.push_back(txtImgPtr);
      try {
        if (rDirection == RenderDirection_Forward)
          tmpPos = book->nextAtom(tmpPos);
        else
          tmpPos = book->prevAtom(tmpPos);
      } catch (const BookError_t &e) {
        if (e.error == BookError_NextAtomNotExists ||
            e.error == BookError_PrevAtomNotExists)
          break;
      }
    }
    endLinePosInd = tmpPos;
    // atomPosInd = getAtomPointersForALine();
    l_atomPosInd = rtn;
    if (rDirection == RenderDirection_Backward) {
      l_atomPosInd.reverse();
      l_atomImages.reverse();
      l_lineAtoms.reverse();
      l_lineGroups.reverse();
    }

    atomPosInd =
        Vector<BookPosIndicator>(l_atomPosInd.begin(), l_atomPosInd.end());
    atomImages = Vector<TextImage *>(l_atomImages.begin(), l_atomImages.end());
    lineAtoms = Vector<BookAtom *>(l_lineAtoms.begin(), l_lineAtoms.end());
    lineGroups = Vector<BookAtomGroup<BookAtom> *>(l_lineGroups.begin(),
                                                   l_lineGroups.end());
    return atomPosInd;
  }
  WORD getMaxHeight() {
    WORD max_h = 0;
    for (auto c : atomImages)
      max_h = MAXVAL(c->image.get_height(), max_h);
    return max_h;
  }
  WORD getMaxBaseLine() {
    WORD max_baseline = 0;
    for (auto c : atomImages)
      max_baseline = MAXVAL(c->base_line, max_baseline);
    return max_baseline;
  }

  inline RGBAImage *get_empty_image(WORD w, WORD h) {
    RGBAImage *rtn = new RGBAImage(w, h);

    fillImageWithPixel(
        *rtn, bookRenderer->getBookRendererFormat()->getTextBackColor());
    return rtn;
  }
  void render_atom_to_canvas(BookDirectionGroup<BookAtom> *dg,
                             RGBAImage *ltr_img, RGBAImage *rtl_img,
                             TextImage *atomImg, int y, WORD &ltr_X_end,
                             WORD &rtl_X_end) {
    auto atomImgW = atomImg->image.get_width();
    auto atomImgH = atomImg->image.get_height();
    if (dg->getDirection() == BookTextDirection_LTR) {
      copyPngToPng(*ltr_img, ltr_X_end, y, atomImg->image, 0, 0, atomImgW - 1,
                   atomImgH - 1);
      ltr_X_end += atomImgW;
    } else if (dg->getDirection() == BookTextDirection_RTL) {
      rtl_X_end -= atomImgW;
      copyPngToPng(*rtl_img, rtl_X_end, y, atomImg->image, 0, 0, atomImgW - 1,
                   atomImgH - 1);
    }
  }
  RGBAImage force_render() {
    MLOG("force_render called.");
    WORD max_height = 0;
    WORD max_baseline = 0;
    WORD max_underbaseline = 0;
    int total_used_x = 0;
    WORD totalw = 0;
    MLOG("iter over atomImages with len:");
    MLOG(atomImages.size());
    for (auto t : atomImages) {
      auto iH = t->image.get_height();
      auto iB = t->base_line;
      totalw += t->image.get_width();
      max_height = MAXVAL(iH, max_height);
      max_baseline = MAXVAL(iB, max_baseline);
      auto under_baseline = iH - iB;
      max_underbaseline = MAXVAL(under_baseline, max_underbaseline);
    }
    auto H = max_underbaseline + max_baseline;
    lineImagePtr = get_empty_image(pageWidth, H);
    RGBAImage *rtl_img = get_empty_image(pageWidth, H);
    RGBAImage *ltr_img = get_empty_image(pageWidth, H);
    WORD rtl_X_start = pageWidth - 1, rtl_X_end = pageWidth - 1;
    WORD ltr_X_start = 0, ltr_X_end = 0;
    BookDirectionGroup<BookAtom> *old_dg = NULL;
    BookAtomGroup<BookAtom> *g = NULL;
    for (WORD i = 0; i < atomImages.size(); i++) {
      auto atomImg = atomImages[i];
      auto atomImgH = atomImg->image.get_height();
      auto atomImgW = atomImg->image.get_width();
      auto atomImgBaseline = atomImg->base_line;
      // std::cout << "W:" << atomImgW << std::endl;
      int y = max_baseline - atomImgBaseline;

      g = lineGroups[i];

      if (g->getType() != ClassName_BookDirectionGroup)
        // think about this line
        continue;
      BookDirectionGroup<BookAtom> *dg = (BookDirectionGroup<BookAtom> *)g;
      if (i == 0 ||
          (old_dg != NULL && dg->getDirection() == old_dg->getDirection())) {
        render_atom_to_canvas(dg, ltr_img, rtl_img, atomImg, y, ltr_X_end,
                              rtl_X_end);
        old_dg = dg;

      } else {
        // Direction changes.
        RGBAImage *canvToCopy;
        int xs_to_copy, xe_to_copy, ys_to_copy, ye_to_copy;
        if (old_dg->getDirection() == BookTextDirection_LTR) {
          canvToCopy = ltr_img;
          xs_to_copy = ltr_X_start;
          xe_to_copy = ltr_X_end;
        } else {
          canvToCopy = rtl_img;
          xs_to_copy = rtl_X_end;
          xe_to_copy = rtl_X_start;
        }
        copyPngToPng(*lineImagePtr,
                     pageWidth - 1 - total_used_x - (xe_to_copy - xs_to_copy),
                     0, *canvToCopy, xs_to_copy, 0, xe_to_copy, H - 1);

        if (old_dg->getDirection() == BookTextDirection_LTR) {
          ltr_X_start = ltr_X_end;
        } else {
          rtl_X_start = rtl_X_end;
        }
        total_used_x += (xe_to_copy - xs_to_copy);

        render_atom_to_canvas(dg, ltr_img, rtl_img, atomImg, y, ltr_X_end,
                              rtl_X_end);
        old_dg = dg;
      }
    }
    if (ltr_X_start != ltr_X_end) {
      // if had rtl
      auto ltr_img_w = ltr_X_end - ltr_X_start;
      if (rtl_X_end != pageWidth) {
        copyPngToPng(*lineImagePtr, pageWidth - 1 - total_used_x - ltr_img_w, 0,
                     *ltr_img, ltr_X_start, 0, ltr_X_end, H - 1);
      } else {
        copyPngToPng(*lineImagePtr, total_used_x, 0, *ltr_img, ltr_X_start, 0,
                     ltr_X_end, H - 1);
      }
    }
    if (rtl_X_start != rtl_X_end) {
      auto rtl_img_w = rtl_X_start - rtl_X_end;
      copyPngToPng(*lineImagePtr, pageWidth - 1 - total_used_x - rtl_img_w, 0,
                   *rtl_img, rtl_X_end, 0, rtl_X_start, H - 1);
    }

    return *lineImagePtr;
  }
  /*
  WORD cX = 0;
  for (auto t : atomImages) {
    auto imageH = t->image.get_height();
    auto imageW = t->image.get_width();
    auto imgbaseline = t->base_line;

    int y = max_baseline - t->base_line;

    copyPngToPng(*lineImagePtr, cX, y, t->image, 0, 0, imageW - 1,
                 imageH - 1);
    cX += imageW;
  }
*/
  RGBAImage render() {
    if (lineImagePtr)
      return *lineImagePtr;
    return force_render();
  }
  Vector<WORD> getEndLinePointer() { return endLinePosInd; }
  Vector<BookAtom *> getBookAtoms() { return lineAtoms; }
  Vector<BookAtomGroup<BookAtom> *> getBookAtomGroups() { return lineGroups; }
};

class MsdBookRendererPage {
protected:
  Book *book;
  WORD pageWidth, pageHeight;
  BookRenderer *bookRenderer;
  RenderDirection renderDirection;
  BookPosIndicator startPagePosInd;
  BookPosIndicator endPagePosInd;
  RGBAImage pageImage;
  // BYTE * pngContent;
  List<MsdBookRendererLine *> lines;
  WORD lineSpace;

public:
  MsdBookRendererPage(Book *book) : book(book) {}
  MsdBookRendererPage(Book *book, WORD pageWidth, WORD pageHeight,
                      BookPosIndicator startPagePosInd,
                      RenderDirection renderDirection,
                      BookRenderer *bookRenderer, WORD lineSpace)
      : book(book), pageWidth(pageWidth), pageHeight(pageHeight),
        bookRenderer(bookRenderer), renderDirection(renderDirection),
        startPagePosInd(startPagePosInd), lineSpace(lineSpace) {
    // std::cout << "Page renderer Called" << std::endl;
    if (book->getBookType() == BookType_PDF) {
      return;
    }
    MLOG("");
    if (startPagePosInd.size() == 0)
      startPagePosInd = book->nextAtom(startPagePosInd);
    MLOG("")
    pageImage = RGBAImage(pageWidth, pageHeight);
    MLOG("filling image with pixel");
    fillImageWithPixel(
        pageImage, bookRenderer->getBookRendererFormat()->getTextBackColor());
    BookPosIndicator tmpPos = getTrueStartPoint();
    if (renderDirection == RenderDirection_Backward)
      endPagePosInd = tmpPos;
    WORD current_height = 0;
    while (true) {
      MLOG("GET A LINE");
      MsdBookRendererLine *line = new MsdBookRendererLine(
          book, tmpPos, pageWidth, bookRenderer, RenderDirection_Forward);
      line->getAtomPointersForALine();
      MLOG("RENDERING LINE");
      RGBAImage lineImage = line->render();
      MLOG("LINE RENDERERD");
      if (line->getMaxHeight() + current_height + lineSpace > pageHeight - 1) {
        break;
      }

      lines.push_back(line);
      copyPngToPng(pageImage, 0, current_height, lineImage, 0, 0,
                   lineImage.get_width() - 1, lineImage.get_height() - 1);
      tmpPos = line->getEndLinePointer();

      current_height += line->getMaxHeight() + lineSpace;
      try {
        // only to check is book finished or not.
        if (renderDirection == RenderDirection_Forward) {
          tmpPos = book->nextAtom(tmpPos);
          tmpPos = book->prevAtom(tmpPos);

        } else {
          tmpPos = book->prevAtom(tmpPos);
          tmpPos = book->nextAtom(tmpPos);
        }
      } catch (const BookError_t &e) {
        MLOG("EXCEPTION HAPPEN. ");
        MLOG(e.error);
        if (e.error == BookError_NextAtomNotExists ||
            e.error == BookError_PrevAtomNotExists)
          break;
      }
    }
    if (renderDirection == RenderDirection_Forward)
      endPagePosInd = tmpPos;
  }
  BookPosIndicator getEndPagePointer() { return endPagePosInd; }

  BookPosIndicator getTrueStartPoint() {
    if (renderDirection == RenderDirection_Forward)
      return startPagePosInd;

    BookPosIndicator tmpPos = startPagePosInd;
    WORD cH = 0;
    List<MsdBookRendererLine *> lines;
    while (true) {
      MsdBookRendererLine *line = new MsdBookRendererLine(
          book, tmpPos, pageWidth, bookRenderer, renderDirection);
      line->getAtomPointersForALine();
      WORD lineMaxHeight = line->getMaxHeight();
      if (cH + lineMaxHeight + lineSpace > pageHeight - 1)
        break;
      lines.push_back(line);
      tmpPos = line->getEndLinePointer();
      cH += lineMaxHeight + lineSpace;
    }
    if (lines.size() > 0) {
      auto last_line = lines.back();
      return last_line->getEndLinePointer();
    }
    throw "No LINE FOUND.";
  }

  RGBAImage getImage() { return pageImage; }
};
class VirtualMsdBookRendererPage {
protected:
  Book *book;
  WORD pageWidth, pageHeight;
  BookRenderer *bookRenderer;
  RenderDirection renderDirection;
  BookPosIndicator startPagePosInd;
  BookPosIndicator endPagePosInd;
  List<VirtualMsdBookRendererLine *> lines;
  WORD lineSpace;

public:
  VirtualMsdBookRendererPage(Book *book, WORD pageWidth, WORD pageHeight,
                             BookPosIndicator startPagePosInd,
                             RenderDirection renderDirection,
                             BookRenderer *bookRenderer, WORD lineSpace)
      : book(book), pageWidth(pageWidth), pageHeight(pageHeight),
        bookRenderer(bookRenderer), renderDirection(renderDirection),
        startPagePosInd(startPagePosInd), lineSpace(lineSpace) {
    // std::cout << "Page renderer Called" << std::endl;

    MLOG("");
    if (startPagePosInd.size() == 0)
      startPagePosInd = book->nextAtom(startPagePosInd);
    MLOG("")
    BookPosIndicator tmpPos = getTrueStartPoint();
    if (renderDirection == RenderDirection_Backward)
      endPagePosInd = tmpPos;
    WORD current_height = 0;
    while (true) {
      MLOG("GET A LINE");
      VirtualMsdBookRendererLine *line = new VirtualMsdBookRendererLine(
          book, tmpPos, pageWidth, bookRenderer, RenderDirection_Forward);
      line->getAtomPointersForALine();
      MLOG("LINE RENDERERD");
      if (line->getMaxHeight() + current_height + lineSpace > pageHeight - 1) {
        break;
      }

      lines.push_back(line);
      tmpPos = line->getEndLinePointer();

      current_height += line->getMaxHeight() + lineSpace;
      try {
        // only to check is book finished or not.
        if (renderDirection == RenderDirection_Forward) {
          tmpPos = book->nextAtom(tmpPos);
          tmpPos = book->prevAtom(tmpPos);

        } else {
          tmpPos = book->prevAtom(tmpPos);
          tmpPos = book->nextAtom(tmpPos);
        }
      } catch (const BookError_t &e) {
        MLOG("EXCEPTION HAPPEN. ");
        MLOG(e.error);
        if (e.error == BookError_NextAtomNotExists ||
            e.error == BookError_PrevAtomNotExists)
          break;
      }
    }
    if (renderDirection == RenderDirection_Forward)
      endPagePosInd = tmpPos;
  }
  BookPosIndicator getEndPagePointer() { return endPagePosInd; }

  BookPosIndicator getTrueStartPoint() {
    if (renderDirection == RenderDirection_Forward)
      return startPagePosInd;

    BookPosIndicator tmpPos = startPagePosInd;
    WORD cH = 0;
    List<VirtualMsdBookRendererLine *> lines;
    while (true) {
      VirtualMsdBookRendererLine *line = new VirtualMsdBookRendererLine(
          book, tmpPos, pageWidth, bookRenderer, renderDirection);
      line->getAtomPointersForALine();
      WORD lineMaxHeight = line->getMaxHeight();
      if (cH + lineMaxHeight + lineSpace > pageHeight - 1)
        break;
      lines.push_back(line);
      tmpPos = line->getEndLinePointer();
      cH += lineMaxHeight + lineSpace;
    }
    if (lines.size() > 0) {
      auto last_line = lines.back();
      return last_line->getEndLinePointer();
    }
    throw "No LINE FOUND.";
  }
};

MsdBookRendererPage
//, BookPosIndicator>
BookRenderer::renderMsdFormatPageAtPointFW(Vector<WORD> pointer) {

  MsdBookRendererPage rtn(book, pageWidth, pageHeight, pointer,
                          RenderDirection_Forward, this,
                          rendererFormat->getLineSpace());
  auto pageEndPointer = rtn.getTrueStartPoint();
  // return std::make_tuple(rtn, pageEndPointer);
  return rtn;
};
MsdBookRendererPage
//, BookPosIndicator>
BookRenderer::renderMsdFormatPageAtPointBack(Vector<WORD> pointer) {
  MsdBookRendererPage rtn(book, pageWidth, pageHeight, pointer,
                          RenderDirection_Backward, this, (WORD)15);
  auto pageEndPointer = rtn.getTrueStartPoint();
  // return std::make_tuple(rtn, pageEndPointer);
  return rtn;
};

Vector<BookPosIndicator> BookRenderer::getPageIndicators() {
  if (book->getBookType() == BookType_PDF ||
      book->getBookType() == BookType_EPUB) {
    Vector<BookPosIndicator> rtn(page_count);
    for (int i = 0; i < page_count; i++) {
      auto bpi = BookPosIndicator({(WORD)-1, (WORD)i});
      rtn.push_back(bpi);
    }
    return rtn;
  }
  auto firstAtom = book->nextAtom(BookPosIndicator());
  List<BookPosIndicator> l;
  BookPosIndicator lastInd = firstAtom;
  while (book->is_last_atom(lastInd) == false) {
    l.push_back(lastInd);
    WORD lineSpace = rendererFormat->getLineSpace();
    auto b =
        VirtualMsdBookRendererPage(book, pageWidth, pageHeight, lastInd,
                                   RenderDirection_Forward, this, lineSpace);
    lastInd = b.getEndPagePointer();
  }
  return Vector<BookPosIndicator>(l.begin(), l.end());
}
#endif