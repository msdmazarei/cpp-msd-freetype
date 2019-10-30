#ifndef _MSD_BOOK_RENDERER_FORMATS_
#define _MSD_BOOK_RENDERER_FORMATS_
#include <png++/png.hpp>

#define PixelColor png::rgba_pixel

class BookRendererFormat {
protected:
  PixelColor textForeColor;
  PixelColor textBackColor;
  PixelColor selectedTextForeColor;
  PixelColor selectedTextBackColor;
  WORD fontSize;
  BYTE *fontBuffer;
  WORD fontBufferLen;

public:
  BookRendererFormat(PixelColor textForeColor, PixelColor textBackColor,
                     PixelColor selectedTextForeColor,
                     PixelColor selectedTextBackColor, WORD fontSize,
                     BYTE *fontBuffer, WORD fontByteSize)
      : textBackColor(textBackColor), textForeColor(textForeColor),
        selectedTextBackColor(selectedTextBackColor),
        selectedTextForeColor(selectedTextForeColor), fontBuffer(fontBuffer),
        fontBufferLen(fontByteSize), fontSize(fontSize) {}

  PixelColor getTextForeColor() { return textForeColor; }
  PixelColor getTextBackColor() { return textBackColor; }
  PixelColor getSelectedTextForeColor() { return selectedTextBackColor; }
  PixelColor getSelectedTextBackColor() { return selectedTextBackColor; }
  BYTE *getFontBuffer() { return fontBuffer; }
  WORD getFontBufferSize() { return fontBufferLen; }
  WORD getFontSize() { return fontSize; }
  WORD getLineSpace() { return 15;}
};

#endif