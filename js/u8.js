  function malloc_8bit(len) {
    return new Uint8Array(len);
  }
  function base64ToBuffer(base64) {
    var binstr = atob(base64);
    var buf = new Uint8Array(binstr.length);
    Array.prototype.forEach.call(binstr, function(ch, i) {
      buf[i] = ch.charCodeAt(0);
    });
    return buf;
  }
  function bufferToBase64(buf) {
    var binstr = Array.prototype.map
                    .call(
                        buf,
                        function(ch) {
                          return String.fromCharCode(ch);
                        })
                    .join('');
    return btoa(binstr);
  }
  // AABrAQAAxmlzUf9K7CnNuqvy++NGfMJU+Bvo5412Wi5jM5/JmmYyDbcxWKNaJV0FF1jpXtSrss3Gm7RUEQ6CdEEhPdyHcOk+oUHh/Gc+AX6X6txrlo84XCrssDv7Mq88VOwY21wCGv5D+/qqOvsp0eYFPHyUddi+YYn5XLuomQ+VsevxswXv9wDpoTrlygvL0EhHZL0fIx6oHHtkxRRzWsVeS3ljO3BkJBGeCdyq1KzyGxCvOzPN41BIRxVcu28iGbqbffUL4RocfyP4KfikGxO1yk7omDI44HlNPTS8X053+stsBayGISuqGlWivnC1czsEXNM2lLOv4vDknk8yFUn9gk6pCHDUsoopVEiaCrzVDhioRKxb845M1y2bCULlBsQzr82jhH8trdR2R94yHOxKxDD2ICOFbPuyBwT07Au5ILqGwz4F8ezZZzO3mVCj4xTT2TT3XqDyEKj2BZQBvrS8RHj6SWnmI9Aa2mmCaXNR7krsKc2JrfK/5UZbxFS9HejvjXZaLkMzn8GaZjIN/zdYuVolXQU5Xulx0quVy8a2slQ2CII1RyEF2od16T6hJdxpc1H6SuwpzKur8vvhRnzDVPob6eeOdl4u


  function copyBufferToHeap(u8buffer) {
    ptr = Module._malloc(u8buffer.length);
    Module.HEAPU8.set(u8buffer, ptr);
    return ptr;
  }
  function freeHeap(ptr) {
    Module._free(ptr);
  }
  getBookFromBuf = Module.cwrap('getBookFromBuf', 'number', ['number', 'number']);
  getBookGroupsCount = Module.cwrap('getBookGroupsCount', 'number', ['number']);
  getBookAtomsCount = Module.cwrap('getBookAtomsCount', 'number', ['number']);
  getRendererFormat = Module.cwrap(
      'getRendererFormat', 'number',
      ['number', 'number', 'number', 'number', 'number', 'number', 'number']);
  getBookRenderer = Module.cwrap(
      'getBookRenderer', 'number', ['number', 'number', 'number', 'number']);
  getRendererFormatTextColor =
      Module.cwrap('getRendererFormatTextColor', 'number', ['number']);
  getImageofPageResult =
      Module.cwrap('getImageofPageResult', 'number', ['number']);
  renderNextPage = Module.cwrap('renderNextPage', 'number', ['number', 'number']);
  getIndicatorPart =
      Module.cwrap('getIndicatorPart', 'number', ['number', 'number']);
  initBookIndicator = Module.cwrap('initBookIndicator', 'number', []);
  BookNextPart = Module.cwrap('BookNextPart', 'number', ['number', 'number']);
  getFontBuffer  = Module.cwrap('getFontBuffer','number',['number']);
  getFontBufferLen = Module.cwrap('getFontBufferLen','number',['number']);
  function color(r, g, b, a) {
    rtn = r;
    rtn = (rtn << 8) + g;
    rtn = (rtn << 8) + b;
    rtn = (rtn << 8) + a;
    return rtn;
  }
  function deserialize_book(b64str) {
    bookbuf = base64ToBuffer(b64str);
    bookbufptr = copyBufferToHeap(bookbuf);
    bookptr = getBookFromBuf(bookbufptr, bookbuf.length);
    return bookptr;
  }
  function getDWORDSize(ptr){
    size =0;
    for(i=3;i>-1;i--){
      size = (size << 8) + Module.HEAPU8[ptr+i];
    }
    return size;
  }
  function extractFromHeapBytes(size, ptr){
    return Module.HEAPU8.slice(ptr,ptr+size);
  }
  async function getFont(url) {
    r = await fetch(url);
    byteArray = await r.arrayBuffer();
    return byteArray;
  }
  function _arrayBufferToBase64( buffer ) {
    var binary = '';
    var bytes = new Uint8Array( buffer );
    var len = bytes.byteLength;
    for (var i = 0; i < len; i++) {
        binary += String.fromCharCode( bytes[ i ] );
    }
    return window.btoa( binary );
}

  async function main() {
    cWhite = color(255, 255, 255, 255);
    cBlack = color(255, 0, 0, 255);
    cBlue = color(0, 0, 255, 255);
    font = await getFont('Zar.ttf');
    font = new Uint8Array(font);
    debugger;
    fontHeapPtr = copyBufferToHeap(font);
    fontSize = 42;
    rendererFormatPtr = getRendererFormat(
        cWhite, cBlack, cWhite, cBlue, fontSize, fontHeapPtr, font.length);
        console.log(getFontBuffer(rendererFormatPtr));
        console.log(getFontBufferLen(rendererFormatPtr));
    book64 =
        'AABrAQAAxmlzUf9K7CnNuqvy++NGfMJU+Bvo5412Wi5jM5/JmmYyDbcxWKNaJV0FF1jpXtSrss3Gm7RUEQ6CdEEhPdyHcOk+oUHh/Gc+AX6X6txrlo84XCrssDv7Mq88VOwY21wCGv5D+/qqOvsp0eYFPHyUddi+YYn5XLuomQ+VsevxswXv9wDpoTrlygvL0EhHZL0fIx6oHHtkxRRzWsVeS3ljO3BkJBGeCdyq1KzyGxCvOzPN41BIRxVcu28iGbqbffUL4RocfyP4KfikGxO1yk7omDI44HlNPTS8X053+stsBayGISuqGlWivnC1czsEXNM2lLOv4vDknk8yFUn9gk6pCHDUsoopVEiaCrzVDhioRKxb845M1y2bCULlBsQzr82jhH8trdR2R94yHOxKxDD2ICOFbPuyBwT07Au5ILqGwz4F8ezZZzO3mVCj4xTT2TT3XqDyEKj2BZQBvrS8RHj6SWnmI9Aa2mmCaXNR7krsKc2JrfK/5UZbxFS9HejvjXZaLkMzn8GaZjIN/zdYuVolXQU5Xulx0quVy8a2slQ2CII1RyEF2od16T6hJdxpc1H6SuwpzKur8vvhRnzDVPob6eeOdl4u';
    bookbuf = base64ToBuffer(book64);
    bookheapPtr = copyBufferToHeap(bookbuf);
    bookPtr = getBookFromBuf(bookheapPtr, bookbuf.length);
    width = 300;
    height = 300;
    bookRendererPtr = getBookRenderer(bookPtr, rendererFormatPtr, width, height);
    bookIndicatorPtr = initBookIndicator();
    firstItem = BookNextPart(bookPtr, bookIndicatorPtr);
    group_i = getIndicatorPart(firstItem, 0);
    atom_i = getIndicatorPart(firstItem, 1);
    NextPage = renderNextPage(bookRendererPtr, firstItem);
    img = getImageofPageResult(NextPage);
    imageSize = getDWORDSize(img);
    pngData = extractFromHeapBytes(imageSize-4,img+4);
    pic ="data:image/png;base64,"+_arrayBufferToBase64(pngData);
    console.log(pngData);
  }