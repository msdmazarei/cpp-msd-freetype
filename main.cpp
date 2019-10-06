// compile:  g++ -I. -I ./defs -I ./renderer/ `pkg-config harfbuzz freetype2
// --cflags` -lpng -lfreetype -lharfbuzz  main.cpp BookReader/*cpp defs/*cpp
// renderer/*cpp -o main -g
#include "BookReader/Book.hpp"
#include "BookReader/BookAtomGroup.hpp"
#include "BookReader/BookAtomText.hpp"
#include "BookReader/BookDirectionGroup.hpp"
#include "libs/utfcpp/source/utf8.h"
#include "renderer/book_renderer.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
bool is_punc(uint32_t mchar) {
  UTF8String chars(L".،؟,.;:()[]/%!#&_ +=«»…");
  int i = 0;
  for (auto c : chars) {
    if (c == mchar)
      return true;
  }
  return false;
}
bool is_punc(UTF8String st) {
  UTF8String chars(L".،؟,.;:()[]/%!#&_ +=«»…");
  int i = 0;
  for (auto c : chars) {
    UTF8String s1({c}); // c + L"";
    if (s1 == st)
      return true;
  }
  return false;
}
BookTextDirection get_dir(BookAtomText *textatom) {
  for (auto c : textatom->decompose()) {
    if (c > 128)
      return BookTextDirection_RTL;
  }
  return BookTextDirection_LTR;
}

std::tuple<BookAtomText, BookAtomText> get_word_atom(string &line,
                                                     string::iterator &itr) {
  auto end_it = line.end();
  UTF8String word;
  while (itr != line.end()) {
    auto next_uchar = utf8::peek_next(itr, end_it);
    if (is_punc(next_uchar)) {
      BookAtomText txt = BookAtomText(word);
      UTF8String puncw({(wchar_t)next_uchar});
      BookAtomText punc_char(puncw);
      utf8::next(itr, end_it);

      return {txt, punc_char};
    } else {
      word += next_uchar;
    }
    utf8::next(itr, end_it);
  }
  return {BookAtomText(word), BookAtomText(L"")};
}

int main(int argc, char **argv) {

  // BookAtomText a = BookAtomText(L"hello");
  // BookAtom &b = a;
  // cout << b.getAtomType() << endl;
  // cout << b.getType() << endl;
  // wcout << L"سلام";
  is_punc(0);
  if (argc != 2) {
    cout << "\nUsage: docsample filename\n";
    return 0;
  }
  const char *test_file_path = argv[1];
  // Open the test file (must be UTF-8 encoded)
  ifstream fs8(test_file_path);
  if (!fs8.is_open()) {
    cout << "Could not open " << test_file_path << endl;
    return 0;
  }

  List<BookAtom *> bookatoms;

  unsigned line_count = 1;
  string line;
  // Play with all the lines in the file
  while (getline(fs8, line)) {
    // check for invalid utf-8 (for a simple yes/no check, there is also
    // utf8::is_valid function)
#if __cplusplus >= 201103L // C++ 11 or later
    auto end_it = utf8::find_invalid(line.begin(), line.end());
#else
    string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
#endif // C++ 11
    if (end_it != line.end()) {

      cout << "Invalid UTF-8 encoding detected at line " << line_count << "\n";
      cout << "This part is fine: " << string(line.begin(), end_it) << "\n";
    }
    // Get the line length (at least for the valid part)
    int length = utf8::distance(line.begin(), end_it);
    auto it_ = line.begin();
    BookAtomText emptystr(L"");
    while (true) {
      auto res = get_word_atom(line, it_);
      auto w = std::get<0>(res);
      auto p = std::get<1>(res);
      if (p == emptystr) {
        if (w != emptystr)
          bookatoms.push_back(w.clone());

        BookAtom a = BookAtom(BookAtomType_Control_NewLine);
        bookatoms.push_back(a.clone());
        break;
      } else {

        bookatoms.push_back(w.clone());
        bookatoms.push_back(p.clone());
      }
    }

    cout << "Length of line " << line_count << " is " << length << "\n";

    // Convert it to utf-16
#if __cplusplus >= 201103L // C++ 11 or later
    u16string utf16line = utf8::utf8to16(line);
#else
    vector<unsigned short> utf16line;
    utf8::utf8to16(line.begin(), end_it, back_inserter(utf16line));
#endif                     // C++ 11
                           // And back to utf-8;
#if __cplusplus >= 201103L // C++ 11 or later
    string utf8line = utf8::utf16to8(utf16line);
#else
    string utf8line;
    utf8::utf16to8(utf16line.begin(), utf16line.end(), back_inserter(utf8line));
#endif // C++ 11
       // Confirm that the conversion went OK:
    if (utf8line != string(line.begin(), end_it))
      cout << "Error in UTF-16 conversion at line: " << line_count << "\n";

    line_count++;
  }

  Vector<BookDirectionGroup<BookAtom>> groups;
  BookTextDirection current_direction = BookTextDirection_LTR;
  List<BookAtom *> *current_dir_atoms = new List<BookAtom *>();
  UTF8String emptystr(L"");
  for (BookAtom *ba : bookatoms) {
    if (ba->getType() == ClassName_BookAtomText) {
      BookAtomText *bat = (BookAtomText *)ba;
      if (is_punc(bat->getText())) {
        current_dir_atoms->push_back(ba);
        continue;
      }
      if (bat->getText() == emptystr)
        continue;
      BookTextDirection atom_dir = get_dir(bat);
      if (current_direction != atom_dir) {
        // dir's changed
        Vector<BookAtom *> v(std::begin(*current_dir_atoms),
                             std::end(*current_dir_atoms));
        BookDirectionGroup<BookAtom> bdg(current_direction, v);
        groups.push_back(bdg);
        current_dir_atoms->clear();
        current_dir_atoms->push_back(ba);
        current_direction = atom_dir;
      } else {
        current_dir_atoms->push_back(ba);
      }

    } else {
      switch (ba->getAtomType()) {
      case BookAtomType_Control_NewLine:
      case BookAtomType_Control_NewPage:
      case BookAtomType_Text:
        current_dir_atoms->push_back(ba);
        break;

      default:
        throw "Unknown Atom Type";
        break;
      }
    }
  }
  Vector<BookAtom *> v(std::begin(*current_dir_atoms),
                       std::end(*current_dir_atoms));
  BookDirectionGroup<BookAtom> bdg(current_direction, v);
  groups.push_back(bdg);

  for (auto c : groups) {
    if (c.getDirection() == BookTextDirection_LTR)
      cout << "LTR " << c.decompose().size() << endl;
    else
      cout << "RTL " << c.decompose().size() << endl;
  }
  Vector<BookAtomGroup<BookAtom> *> cgr;
  for (int i = 0; i < groups.size(); i++) {
    cgr.push_back(&groups[i]);
  }
  // for (auto c : groups) {
  // cgr.push_back(&c);
  // }
  Book b(BookType_MSDFORMAT, cgr);

  // read font file
  ifstream ifs("/home/msd/.fonts/Zar.ttf", ios::binary | ios::ate);
  ifstream::pos_type pos = ifs.tellg();

  // What happens if the OS supports really big files.
  // It may be larger than 32 bits?
  // This will silently truncate the value/
  int fontlength = pos;

  // Manuall memory management.
  // Not a good idea use a container/.
  BYTE *fontPtr = new BYTE[fontlength];
  ifs.seekg(0, ios::beg);
  ifs.read((char *)fontPtr, fontlength);

  // No need to manually close.
  // When the stream goes out of scope it will close the file
  // automatically. Unless you are checking the close for errors
  // let the destructor do it.
  ifs.close();

  auto foreTextC = PixelColor(255, 255, 255, 255);
  auto backC = PixelColor(0, 0, 0, 255);
  auto selectedTextBackColor = PixelColor(0, 0, 255, 255);
  BookRendererFormat bookFormat(foreTextC, backC, foreTextC,
                                selectedTextBackColor, 42, fontPtr, fontlength);

  BookRenderer bookRenderer(&b, &bookFormat, 300, 300);
  BookPosIndicator bpi = b.nextAtom(BookPosIndicator());
  auto rtn = bookRenderer.renderMsdFormatPageAtPointFW(bpi);
  rtn.getImage().write("pageImage.png");
  
  return 0;
}