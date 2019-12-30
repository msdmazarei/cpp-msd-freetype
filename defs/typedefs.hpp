#ifndef _H_MSD_TYEPDEFS_
#define _H_MSD_TYEPDEFS_
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#define GetByteN(x, N) ((x >> (N * 8)) & 0xff);
#define MLOG(x)                       ;//                                         \
  // std::cout << __FILE__ << ":" << __LINE__ << " " << x << std::endl;
#define MLOG2(x, y)                  ;//                                          \
                                   //                                            \
  // std::cout << __FILE__ << ":" << __LINE__ << " " << x << " " << y << std::endl;

#define MLOG3(x, y, z)            ;//                                             \
                                //                                               \
  // std::cout << __FILE__ << ":" << __LINE__ << " " << x << " " << y << " " << z \
            // << std::endl;

#define MLOG4(x, y, z, a)  ;//                                                    \
                              //                                                 \
  // std::cout << __FILE__ << ":" << __LINE__ << " " << x << " " << y << " " << z \
            // << " " << a << std::endl;

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;
typedef DWORD VOICEDURATION;
typedef unsigned long ULONG;
typedef short INT16;
enum BookTextDirection { BookTextDirection_RTL = 0, BookTextDirection_LTR = 1 };
enum BookType {
  BookType_MSDFORMAT = 0,
  BookType_PDF = 1,
  BookType_Voice = 2,
  BookType_EPUB = 4
};
enum BookAtomType {
  BookAtomType_Text = 0,
  BookAtomType_Picture = 1,
  BookAtomType_Voice = 2,
  BookAtomType_PDF = 3,
  BookAtomType_EPUB = 4,
  BookAtomType_XPS = 5,
  BookAtomType_Binary = 6,

  BookAtomType_Control_NewLine = 100,
  BookAtomType_Control_NewPage = 101

};
enum RenderDirection { RenderDirection_Forward, RenderDirection_Backward };

typedef std::basic_string<wchar_t> UTF8String;
typedef std::basic_string<char> String;
template <typename T> using Vector = std::vector<T>;
template <typename T> using List = std::list<T>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename V> using Set = std::set<V>;
template <typename... K> using Tuple = std::tuple<K...>;

#define MAXVAL(a, b) a > b ? a : b;

typedef Vector<WORD> BookPosIndicator;
WORD utf8clen(UTF8String &str);
Vector<BYTE> UTF8toByteBuffer(UTF8String str);
List<BYTE> DWORD2Bytes(DWORD);
WORD getWORD(BYTE *buf);
DWORD getDWORD(BYTE *buf);
long ceil(double num);
#endif