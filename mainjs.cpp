#include "utils/jsreader.hpp"
#include "Book.hpp"
#include "book_player.hpp"
#include "renderer/book_renderer.hpp"
// #include "BookReader/MsdRandomReader.hpp"
#include "BookReader/MsdLazyBook.hpp"
#include <emscripten.h>
#include <sstream>
// export
// PKG_CONFIG_PATH=/home/msd/projects/mpg123-1.25.12/wasmbuild/lib/pkgconfig

// em++ -std=c++1z  -I BookReader/ -I libs -I. -I ./defs -I ./renderer/ -I
// ./player/ `pkg-config harfbuzz freetype2 libmpg123 --cflags` -I
// /home/msd/projects/mupdf/include
// -L/home/msd/projects/mpg123-1.25.12/wasmbuild/lib  -llibmpg123 -lm  -s
// DISABLE_EXCEPTION_CATCHING=0 -s USE_LIBPNG=1 -s WASM=1  -s USE_FREETYPE=1
// mainjs.cpp BookReader/*cpp defs/*cpp renderer/*cpp
// /home/msd/projects/mupdf/build/wasm/release/libmupdf.a
// /home/msd/projects/mupdf/build/wasm/release/libmupdf-third.a
// /home/msd/projects/mupdf/build/wasm/release/source/fitz/harfbuzz.o -o
// _build/reader.html  -s
// EXPORTED_FUNCTIONS='["_aTestFunc","_getBookFromBuf","_getBookAtomsCount","_getBookGroupsCount","_getRendererFormat","_getBookRenderer","_getRendererFormatTextColor","_getIndicatorPart","_renderNextPage","_getImageofPageResult","_initBookIndicator","_BookNextPart","_getFontBuffer","_getFontBufferLen","_deleteRenderedPage","_getBookType","_is_first_atom","_is_last_atom","_getBookIndicatorPartOfPageResult","_deleteBytePoniter","_deleteBookPosIndicator","_getBookProgress",
// "_getBookTotalAtoms","_renderBackPage","_gotoBookPosIndicator","_getBookPosIndicators","_getBookContentAt","_getBookContentLength","_renderNextPages","_getBookPlayer","_getVoiceDuration","_deleteBookPlayer","_getVoiceAtomWrapper","_deleteVoiceAtomWrapper","_getVoiceSampleRate","_getVoiceChannelsCount","_get10Seconds","_getFirstAtom","_getLastAtom","_getVoiceAtomWrapperDuration","_renderDocPage"]'
// -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s
// ALLOW_MEMORY_GROWTH=1




//em++  -v -std=c++1z  -I /home/msd/projects/mpg123-1.25.12/build/include -IBookReader -I libs -I. -I ./defs -I ./renderer/ -I ./player/ `pkg-config harfbuzz freetype2 libmpg123 --cflags` -I /home/msd/tmp/mupdf/include -L/home/msd/projects/mpg123-1.25.12/build/lib  -llibmpg123 -lm  -s DISABLE_EXCEPTION_CATCHING=0 -s USE_LIBPNG=1   -s USE_FREETYPE=1 mainjs.cpp BookReader/*cpp defs/*cpp renderer/*cpp /home/msd/tmp/mupdf/build/wasm/release/libmupdf.a /home/msd/tmp/mupdf/build/wasm/release/libmupdf-third.a  -o _build/reader.html  -s EXPORTED_FUNCTIONS='["_aTestFunc","_getBookFromBuf","_getBookAtomsCount","_getBookGroupsCount","_getRendererFormat","_getBookRenderer","_getRendererFormatTextColor","_getIndicatorPart","_renderNextPage","_getImageofPageResult","_initBookIndicator","_BookNextPart","_getFontBuffer","_getFontBufferLen","_deleteRenderedPage","_getBookType","_is_first_atom","_is_last_atom","_getBookIndicatorPartOfPageResult","_deleteBytePoniter","_deleteBookPosIndicator","_getBookProgress", "_getBookTotalAtoms","_renderBackPage","_gotoBookPosIndicator","_getBookPosIndicators","_getBookContentAt","_getBookContentLength","_renderNextPages","_getBookPlayer","_getVoiceDuration","_deleteBookPlayer","_getVoiceAtomWrapper","_deleteVoiceAtomWrapper","_getVoiceSampleRate","_getVoiceChannelsCount","_get10Seconds","_getFirstAtom","_getLastAtom","_getVoiceAtomWrapperDuration","_renderDocPage","_getMsdLazyBook","_deleteMsdLazyBook"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY -s 'ASYNCIFY_IMPORTS=["js_do_fetch"]'
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
extern BYTE *renderDocPage(BookRenderer *renderer, BookPosIndicator *ind,
                           WORD zoom, WORD rotate) {
  auto pageRtn = renderer->renderDocPage(*ind, zoom, rotate);
  auto s = std::get<0>(pageRtn);
  auto b = std::get<1>(pageRtn);
  DWORD rtnbuffersizer = s + 4;
  BYTE *rtn = new BYTE[rtnbuffersizer];
  for (int i = 0; i < 4; i++)
    rtn[i] = GetByteN(rtnbuffersizer, i);

  memcpy(rtn + 4, b, s);
  delete b;

  return rtn;
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
extern WORD getBookType(Book *b) { return b->getBookType(); }
extern WORD deleteBookPosIndicator(BookPosIndicator *p) {
  delete p;
  return 1;
}
extern WORD deleteRenderedPage(MsdBookRendererPage *p) {
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

extern WORD deleteBytePoniter(BYTE *p) {
  delete p;
  return 1;
}
extern DWORD getBookTotalAtoms(Book *b) { return b->getTotalAtoms(); }
extern DWORD getBookProgress(Book *b, BookPosIndicator *ind) {
  return b->progress(*ind);
}
extern BookPosIndicator *gotoBookPosIndicator(WORD group, WORD atom) {
  return new BookPosIndicator({group, atom});
}
extern BYTE *getBookPosIndicators(BookRenderer *bookRenderer) {
  auto v = bookRenderer->getPageIndicators();
  BYTE *buf = new BYTE[v.size() * 8 + 4];
  auto vsize = v.size() * 8 + 4;
  for (int i = 0; i < 4; i++)
    buf[i] = GetByteN(vsize, i);

  for (int i = 0; i < v.size(); i++) {
    if (v[i].size() == 2) {
      WORD g = v[i][0];
      WORD a = v[i][1];
      // std::cout << "g:" << g << "a:" << a << std::endl;
      for (int j = 0; j < 4; j++)
        buf[4 + i * 8 + j] = GetByteN(g, j);
      for (int j = 0; j < 4; j++)
        buf[4 + i * 8 + 4 + j] = GetByteN(a, j);

    } else {
      MLOG("UNEXCEPTED BOOK POSITION INDICATOR");
    }
  }
  return buf;
}
extern WORD getBookContentLength(Book *b) { return b->getBookContent().size(); }
void serialize_bytes(BYTE *buf, DWORD dword, int from_, int to_) {
  for (int i = from_; i < to_; i++)
    buf[i] = GetByteN(dword, i);
}
extern BYTE *getBookContentAt(Book *b, WORD index) {
  if (b->getBookContent().size() > index) {
    auto i = b->getBookContent()[index];
    UTF8String s;
    for (auto g : i.getTitle()) {
      for (auto c : g->decompose()) {
        if (c->getAtomType() == BookAtomType_Text) {
          auto txt = (BookAtomText *)c;
          s += (txt->getText());
        } else if (c->getAtomType() == BookAtomType_Control_NewLine) {
          s += L"\n";
        }
      }
    }
    auto bys = UTF8toByteBuffer(s);
    DWORD tlen = 4 + 4 + 4 + 2 + bys.size();
    BYTE *rtn = new BYTE[tlen];
    for (int i = 0; i < 4; i++)
      rtn[i] = GetByteN(tlen, i);
    auto pos = i.getPos();
    WORD gindex = pos[0];
    WORD aindex = pos[1];
    WORD parentindex = i.getParentIndex();
    serialize_bytes(rtn + 4, gindex, 0, 4);
    serialize_bytes(rtn + 4 + 4, aindex, 0, 4);
    serialize_bytes(rtn + 4 + 4 + 4, parentindex, 0, 2);
    for (int i = 0; i < bys.size(); i++)
      rtn[4 + 4 + 4 + 2 + i] = bys[i];
    return rtn;

  } else {
    return NULL;
  }
}

extern BYTE *renderNextPages(BookRenderer *renderer, BookPosIndicator *ind,
                             WORD pageCount) {
  // std::cout << "renderNextPages Called. pagecount:" << pageCount <<
  // std::endl;
  List<BYTE *> bytes;
  BookPosIndicator in = *ind;
  if (renderer->getBook()->is_last_atom(*ind))
    return NULL;
  for (int i = 0; i < pageCount; i++) {
    // std::cout<< "render page :" << i << std::endl;
    auto ip = renderer->renderMsdFormatPageAtPointFW(in);
    in = ip.getEndPagePointer();
    bytes.push_back(getImageofPageResult(&ip));
    if (renderer->getBook()->is_last_atom(in))
      break;
  }
  BYTE *rtn = new BYTE[4 + 4 * bytes.size()];
  serialize_bytes(rtn, bytes.size() * 4 + 4, 0, 4);
  int i = 0;
  for (auto p : bytes) {
    serialize_bytes(rtn + 4 + i * 4, (DWORD)p, 0, 4);
    i++;
  }
  // std::cout <<" it retuns/" << std::endl;
  return rtn;
}

extern BookPlayer *getBookPlayer(Book *book) { return new BookPlayer(book); }
extern DWORD getVoiceDuration(BookPlayer *bookPlayer, BookPosIndicator *f,
                              BookPosIndicator *t) {
  return bookPlayer->getDurationFromTo(*f, *t);
}
extern int deleteBookPlayer(BookPlayer *bookPlayer) {
  delete bookPlayer;
  return 0;
}
extern MpgWrapper2 *getVoiceAtomWrapper(BookPlayer *bookPlayer,
                                        BookPosIndicator *p) {
  return bookPlayer->getVoiceAtomWrapper(*p);
}
extern int deleteVoiceAtomWrapper(MpgWrapper2 *p) {
  delete p;
  return 0;
}
extern long getVoiceSampleRate(MpgWrapper2 *w) { return w->getRate(); }
extern int getVoiceChannelsCount(MpgWrapper2 *w) { return w->getChannels(); }
extern VOICEDURATION getVoiceAtomWrapperDuration(MpgWrapper2 *w) {
  return w->getTotalSecods() * 1000;
}
extern BYTE *get10Seconds(MpgWrapper2 *w, VOICEDURATION fromMilliSecond) {
  // std::cout << "get10Seconds Called. fromMilli:" << fromMilliSecond
  // << std::endl;
  auto r = w->get10Second(fromMilliSecond);
  auto buf = std::get<0>(r);
  auto buflen = std::get<1>(r) + 4 /*for spec size of buf*/;
  // std::cout << "get10Seconds buflen:" << buflen << "and buf:" << (ULONG)buf
  // << std::endl;
  BYTE *rtn = new BYTE[buflen];
  for (int i = 0; i < 4; i++)
    rtn[i] = GetByteN(buflen, i);
  memcpy(rtn + 4, buf, buflen - 4);
  // std::cout << "memory copy done" << std::endl;
  // dont free buf cause it is refer to pcm buffer;
  // free(buf);
  // std::cout << "get10Seconds Done. returns rtn: " << (ULONG)rtn
  // << "buflen:" << buflen << std::endl;
  return rtn;
}
extern BookPosIndicator *getFirstAtom(Book *b) {
  return new BookPosIndicator(b->getFirstAtom());
}
extern BookPosIndicator *getLastAtom(Book *b) {
  return new BookPosIndicator(b->getLastAtom());
}
extern WORD getPageCount(BookRenderer *p){
  return p->getPageCount();
}
extern int aTestFunc(int i) { return i + 3285; }
EM_JS(void, do_fetch, (), {
  Asyncify.handleSleep(function(wakeUp) {
    out("waiting for a fetch");
    fetch("a.html").then(response => {
      out("got the fetch response");
      // (normally you would do something with the fetch here)
      wakeUp();
    });
  });
});

extern MsdLazyBook*  getMsdLazyBook(DWORD id,DWORD len){
  JsMsdRandomReader* reader = new JsMsdRandomReader(id,len);
  MsdLazyBook * rtn = new MsdLazyBook(reader);
  return rtn;
}
extern int deleteMsdLazyBook(MsdLazyBook* book){
  delete  book;
  return 0;
}


EM_JS(DWORD , js_do_fetch_1, (), {
  return Asyncify.handleSleep(function(wakeUp) {
    debugger;
    out("Wasm: waiting for a fetch");
    out("id:",id);
    let k = msd_js_function_read_bytes(0,10);
    k.then(res =>{
      debugger;
      wakeUp(0);
      out("WASM: k resolved.");
      return;
      

        wakeUp(l);
    //   wakeUp(res);
    }, rej => {
      out("WASM: k rejected");
      wakeUp(0);
    });
    // await k;
    // fetch("a.html").then(response => {
    //   out("got the fetch response");
    //   // (normally you would do something with the fetch here)
    //   wakeUp();
    // });
  });
});

extern int asyncFuncTest(){
  puts("asyncFuncTest Called.");
  js_do_fetch_1();
  puts("After do_fetch() call.");
  return 0;
};

extern int asyncFuncTest2(){
  puts("asyncFuncTest2 Called.");
  js_do_fetch(0,0,0,0);
  puts("aftre js_do_fetch() call.");
  puts("asyncFuncTest2 Done.");
  return 0;
};

extern int nestedAsyncCall(){
  puts("nestedAsyncCall Called.");
  asyncFuncTest2();
  puts("nestedAsyncCall Done");
  return 0;
}

}

