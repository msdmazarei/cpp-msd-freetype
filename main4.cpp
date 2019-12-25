#include "MsdLazyBook.hpp"
#include "player/book_player.hpp"
#include "renderer/book_renderer.hpp"
#include <iostream>
// g++ -IBookReader -I. -I ./defs -I ./renderer/ `pkg-config harfbuzz freetype2
// --cflags` -lpng -lfreetype -lharfbuzz  main4.cpp BookReader/*cpp defs/*cpp
// renderer/*cpp  /home/msd/projects/mupdf/build/debug/libmupdf.a
// /home/msd/projects/mupdf/build/debug/libmupdf-third.a -o main -g
//

int main(void) {
    Vector<BYTE> encKey(0);
    auto a1 = new LazyBookAtomVoice((MsdRandomReader *)NULL,0,0,encKey,100,(VoiceAtomBinFormat)2,0,0);
    auto b1 = dynamic_cast<BookAtomVoiceBase*>(a1);
    auto c1 = dynamic_cast<BookAtom*>(a1);

    auto a21 = dynamic_cast<LazyBookAtomVoice*>(b1);
    auto a22 = dynamic_cast<LazyBookAtomVoice*>(c1);

    auto b21 = dynamic_cast<BookAtomVoiceBase*>(a1);
    auto b22 = dynamic_cast<BookAtomVoiceBase*>(c1);

    auto c11 = dynamic_cast<BookAtom*>(b1);
    
    BookAtom *bookatom = dynamic_cast<BookAtom*>(a1);
    BookAtomVoiceBase* bavb  = dynamic_cast<BookAtomVoiceBase*>(a1);
    std::cout << bavb->getDuration() << std::endl;
    
    BookAtom *a2= dynamic_cast<BookAtom*>(bavb);
    LazyBookAtomVoice* lbav = dynamic_cast<LazyBookAtomVoice*>(bavb);
    lbav->getType();
    std::cout << bookatom->getType() << std::endl; 
    std::cout << dynamic_cast<BookAtomVoiceBase*>(bookatom)->getDuration() << std::endl;
    std::cout << dynamic_cast<LazyBookAtomVoice*>(bookatom)->getType() << std::endl;
  const char *filename = "/home/msd/projects/jjp_ebook_creator/outpdf1.msd";
  FileMsdRandomReader *reader = new FileMsdRandomReader(filename);
  MsdLazyBook bk(reader);
  BookPlayer player(&bk);
  auto a = bk.getFirstAtom();
  while (true) {
      
    auto atm = bk.getAtom(a);
    
       if (atm->getAtomType() == BookAtomType_Voice) {
         auto a1 = dynamic_cast<LazyBookAtomVoice*>(atm);
          std::cout << (dynamic_cast<BookAtom*>(atm))->getType() << std::endl;
    std::cout << (dynamic_cast<BookAtomVoiceBase *>(atm))->getDuration() << std::endl;

      auto k1 = player.getAtomDuration(atm);
      auto k2 = player.getVoiceAtomWrapper(a);
      auto c = k2->getChannels();
      auto r = k2->getRate();
      auto s = k2->getTotalSecods();
      auto h = k2->get10Second(0);
    }

    if(atm->getAtomType()==BookAtomType_PDF){
      BookRendererFormat *fmt = new BookRendererFormat(100);
      Book * b = (Book*)&bk;
     BookRenderer *r = new BookRenderer(b,fmt,500,500);
     auto pos = BookPosIndicator({(WORD)-1,0});
     auto res = r->renderDocPage(pos,100,0);
FILE *pFile;
  pFile = fopen("file2.png", "wb");
  fwrite(std::get<1>(res), std::get<0>(res), 1, pFile);
  fclose(pFile);
    }
    if(bk.is_last_atom(a)==false)
    a = bk.nextAtom(a);
    else break;
  }

  std::cout << "b inited" << std::endl;
}