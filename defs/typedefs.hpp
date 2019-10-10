#ifndef _H_MSD_TYEPDEFS_
#define _H_MSD_TYEPDEFS_
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#define GetByteN(x, N) ((x >> (N * 8)) & 0xff);
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;

enum BookTextDirection { BookTextDirection_RTL = 0, BookTextDirection_LTR = 1 };
enum BookType { BookType_MSDFORMAT = 0, BookType_PDF = 1, BookType_Voice = 2 };
enum BookAtomType {
  BookAtomType_Text = 0,
  BookAtomType_Picture = 1,
  BookAtomType_Voice = 2,

  BookAtomType_Control_NewLine = 100,
  BookAtomType_Control_NewPage = 101

};
enum RenderDirection { RenderDirection_Forward, RenderDirection_Backward };

typedef std::basic_string<wchar_t> UTF8String;
template <typename T> using Vector = std::vector<T>;
template <typename T> using List = std::list<T>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename V> using Set = std::set<V>;

#define MAXVAL(a, b) a > b ? a : b;

typedef Vector<WORD> BookPosIndicator;
WORD utf8clen(UTF8String &str);
Vector<BYTE> UTF8toByteBuffer(UTF8String str);
List<BYTE> DWORD2Bytes(DWORD);
#endif