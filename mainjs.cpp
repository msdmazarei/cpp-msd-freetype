#include "Book.hpp"
#include "renderer/book_renderer.hpp"
// #include <emscripten.h>
#include <sstream>

// em++ -std=c++1z -I BookReader/ -I libs -I. -I ./defs -I ./renderer/
// `pkg-config harfbuzz freetype2 --cflags` -s USE_LIBPNG=1 -s WASM=1  -s
// USE_FREETYPE=1  -s USE_HARFBUZZ=1 mainjs.cpp BookReader/*cpp defs/*cpp
// renderer/*cpp -o _build/reader.html  -s
// EXPORTED_FUNCTIONS='["_aTestFunc","_getBookFromBuf","_getBookAtomsCount","_getBookGroupsCount","_getRendererFormat","_getBookRenderer","_getRendererFormatTextColor","_getIndicatorPart","_renderNextPage","_getImageofPageResult","_initBookIndicator","_BookNextPart"]'
// -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s
// DISABLE_EXCEPTION_CATCHING=0
extern "C" {
extern Book *getBookFromBuf(BYTE *buf, DWORD len) {
  return Book::deserialize(len, buf);
}
extern int getBookGroupsCount(Book *book) { return book->decompose().size(); }
extern int getBookAtomsCount(Book *book) {
  int c = 0;
  for (auto g : book->decompose()) {
    for (auto a : g->decompose())
      c++;
  }
  return c;
}

extern BYTE *getFontBuffer(BookRendererFormat *fmt) {
  return fmt->getFontBuffer();
}
extern DWORD getFontBufferLen(BookRendererFormat *fmt) {
  return fmt->getFontBufferSize();
}
extern BookRendererFormat *
getRendererFormat(DWORD textForeColor, DWORD textBackColor,
                  DWORD selectedTextForeColor, DWORD selectedTextBackColor,
                  WORD fontSize, BYTE *fontBuffer, WORD fontByteSize) {
  BYTE r = GetByteN(textForeColor, 3);
  BYTE g = GetByteN(textForeColor, 2);
  BYTE b = GetByteN(textForeColor, 1);
  BYTE a = GetByteN(textForeColor, 0);

  PixelColor _textForeColor(r, g, b, a);
  r = GetByteN(textBackColor, 3);
  g = GetByteN(textBackColor, 2);
  b = GetByteN(textBackColor, 1);
  a = GetByteN(textBackColor, 0);

  PixelColor _textBackColor(r, g, b, a);
  r = GetByteN(selectedTextForeColor, 3);
  g = GetByteN(selectedTextForeColor, 2);
  b = GetByteN(selectedTextForeColor, 1);
  a = GetByteN(selectedTextForeColor, 0);

  PixelColor _selectedTextForeColor(r, g, b, a);
  r = GetByteN(selectedTextBackColor, 3);
  g = GetByteN(selectedTextBackColor, 2);
  b = GetByteN(selectedTextBackColor, 1);
  a = GetByteN(selectedTextBackColor, 0);

  PixelColor _selectedTextBackColor(r, g, b, a);

  return new BookRendererFormat(_textForeColor, _textBackColor,
                                _selectedTextForeColor, _selectedTextBackColor,
                                fontSize, fontBuffer, fontByteSize);
}
extern DWORD getRendererFormatTextColor(BookRendererFormat *format) {
  auto c = format->getTextForeColor();
  DWORD rtn = c.red;
  rtn = (rtn << 8) + c.green;
  rtn = (rtn << 8) + c.blue;
  rtn = (rtn << 8) + c.alpha;
  return rtn;
}
extern BookRenderer *getBookRenderer(Book *book, BookRendererFormat *format,
                                     WORD width, WORD height) {
  return new BookRenderer(book, format, width, height);
}

extern BookPosIndicator *initBookIndicator() { return new BookPosIndicator(); }
extern BookPosIndicator *BookNextPart(Book *book, BookPosIndicator *ind) {
  auto rtn = book->nextAtom(*ind);
  return new BookPosIndicator(rtn);
}
extern int getIndicatorPart(BookPosIndicator *ind, int part) {
  if (part < 0)
    return -1;
  if (ind->size() > part)
    return (*ind)[part];
  return -1;
}
extern MsdBookRendererPage *renderNextPage(BookRenderer *renderer,
                                           BookPosIndicator *ind) {

  auto rtn = renderer->renderMsdFormatPageAtPointFW(*ind);
  return new MsdBookRendererPage(rtn);
}
extern MsdBookRendererPage *renderBackPage(BookRenderer *renderer,
                                           BookPosIndicator *ind) {

  auto rtn = renderer->renderMsdFormatPageAtPointBack(*ind);
  return new MsdBookRendererPage(rtn);
}
extern BYTE is_first_atom(Book *b, BookPosIndicator *ind) {
  return b->is_first_atom(*ind);
}
extern BYTE is_last_atom(Book *b, BookPosIndicator *ind) {
  return b->is_last_atom(*ind);
}
extern WORD getBookType(Book *b){
  return b->getBookType();
}
extern WORD deleteBookPosIndicator(BookPosIndicator *p){
  delete p;
  return 1;
}
extern WORD deleteRenderedPage(MsdBookRendererPage *p){
  delete p;
  return 1;
}
extern BYTE *getImageofPageResult(MsdBookRendererPage *p) {
  auto img = p->getImage();
  // auto img = RGBAImage(300,300);
  // fillImageWithPixel(img,PixelColor(255,0,0,255));
  std::stringstream s;
  img.write_stream(s);
  s.seekg(0, std::ios::end);
  int size = s.tellg();
  BYTE *rtn = new BYTE[size + 4];
  MLOG2("total size:", size + 4);
  auto size2 = size + 4;
  rtn[0] = GetByteN(size2, 0);
  rtn[1] = GetByteN(size2, 1);
  rtn[2] = GetByteN(size2, 2);
  rtn[3] = GetByteN(size2, 3);
  s.seekg(0, std::ios::beg);
  s.read((char *)(rtn + 4), size);
  return rtn;
}
extern BookPosIndicator *
getBookIndicatorPartOfPageResult(MsdBookRendererPage *p) {
  return new BookPosIndicator(p->getEndPagePointer());
}

extern WORD deleteBytePoniter(BYTE *p){
  delete p;
  return 1;
}
extern int aTestFunc(int i) { return i + 3285; }
}