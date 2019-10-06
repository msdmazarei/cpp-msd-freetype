#include "typedefs.hpp"
WORD utf8clen(UTF8String &str) { return str.length() * 2; }
Vector<BYTE> UTF8toByteBuffer(UTF8String str) {

  // https://en.wikipedia.org/wiki/UTF-8
  List<BYTE> bytes;
  for (wchar_t c : str)
    if (c < 0x7f)
      bytes.push_back(c);
    else if (c < 0x7ff) {
      BYTE b0 = (c & 0b111111) | 0b10000000;
      BYTE b1 = ((c & 0b1111111111000000) >> 6) | 0b11000000;
      bytes.push_back(b1);
      bytes.push_back(b0);
    } else if (c < 0xffff) {
      BYTE b0 = (c & 0b111111) | 0b10000000;
      BYTE b1 = ((c & 0b11111111000000) >> 6) | 0b10000000;
      BYTE b2 = ((c & 0b1111000000000000) >> 12) | 0b11100000;
      bytes.push_back(b2);
      bytes.push_back(b1);
      bytes.push_back(b0);
    }
  Vector<BYTE> v(bytes.begin(), bytes.end());

  return v;
}