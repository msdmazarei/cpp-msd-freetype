#ifndef _MSDLAZYBOOK_
#define _MSDLAZYBOOK_
#include "Book.hpp"
#include "BookReader/MsdRandomReader.hpp"
#include "LazyBookAtom.hpp"
#include "LazyBookAtomBinary.hpp"
#include "LazyBookAtomVoice.hpp"

class MsdLazyBook : public Book {
protected:
  MsdRandomReader *randomReader;
  Vector<BYTE> encKey;
  DWORD atomTableOffset, atomTableLen;
  DWORD groupTableOffset, groupTableLen;
  DWORD contentTableOffset, contentTableLen;
  DWORD atomindicesTableOffset, atomindicesTableLen;

  Vector<DWORD> atomIndices;
  Vector<DWORD> atomLengths;
  Vector<BookAtomType> atomTypes;
  Vector<BookAtom *> atoms;

public:
  ClassName getType() override { return ClassName_MsdLazyBook; }
  DWORD getTableSize(BYTE *tableBuf) {
    BYTE *tableSizeBytes = new BYTE[4];
    auto encKeyLen = encKey.size();
    for (int i = 0; i < 4; i++) {
      if (encKeyLen > 0)
        tableSizeBytes[i] = tableBuf[i] ^ encKey[i % encKeyLen];
      else
        tableSizeBytes[i] = tableBuf[i];
    }
    auto rtn = getDWORD(tableSizeBytes);
    free(tableSizeBytes);
    return rtn;
  }
  BYTE *decryptbuf(BYTE *buf, DWORD offset, DWORD len) {
    auto encKeyLen = encKey.size();

    if (encKeyLen == 0)
      return buf;
    BYTE *rtn = new BYTE[len];
    memcpy(rtn, buf, len);
    if (encKeyLen > 0)
      for (DWORD i = 0; i < len; i++) {
        rtn[i] = rtn[i] ^ encKey[(i + offset) % encKeyLen];
      }
    return rtn;
  }
  void loadAtoms() {
    for (DWORD i = 0; i < atomIndices.size(); i++) {
      auto atomOffset =
          atomIndices[i] + atomTableOffset + 4 /*atom table len bytes*/;
      switch (atomTypes[i]) {
      case BookAtomType_Control_NewLine:
      case BookAtomType_Control_NewPage:
      case BookAtomType_Text: {
        auto rbytes = randomReader->read(atomOffset, atomLengths[i]);
        if (std::get<0>(rbytes) != atomLengths[i])
          throw "BAD FILE READ";
        auto buf = std::get<1>(rbytes);
        auto decryptedBuf =
            decryptbuf(buf, atomOffset - atomTableOffset, atomLengths[i]);
        if (decryptedBuf != buf)
          free(buf);
        atoms[i] = deserialize_atom(decryptedBuf, 0);
        free(decryptedBuf);
      } break;
      case BookAtomType_Voice: {
        auto rbytes = randomReader->read(atomOffset, 10);
        if (std::get<0>(rbytes) != 10)
          throw "BAD FILE READ";
        auto buf = std::get<1>(rbytes);
        auto decryptedBuf = decryptbuf(buf, atomOffset - atomTableOffset, 10);
        if (decryptedBuf != buf)
          free(buf);
        auto audioFormat = decryptedBuf[5];
        auto audioDuration = getDWORD(decryptedBuf + 6);
        free(decryptedBuf);
        auto aatom = new LazyBookAtomVoice(
            randomReader, atomOffset, atomLengths[i], encKey, audioDuration,
            (VoiceAtomBinFormat)audioFormat, 10, atomOffset - atomTableOffset);
            
            // MLOG(aatom->getDuration());
            // MLOG(aatom->getAtomType());
            // MLOG(aatom->getType());
            BookAtom *baatom =dynamic_cast<BookAtom*>( aatom);
            MLOG(dynamic_cast<LazyBookAtomVoice*>(baatom)->getBufferLength());
            // MLOG(baatom->getDuration());
        atoms[i] = baatom;

      }

      break;
      case BookAtomType_PDF:
        atoms[i] =dynamic_cast<BookAtom*>( new LazyBookAtomPDF(randomReader, atomOffset, atomLengths[i],
                                       encKey, 5, atomOffset - atomTableOffset));
        break;
      case BookAtomType_EPUB:
        atoms[i] = dynamic_cast<BookAtom*>(
            new LazyBookAtomEPUB(randomReader, atomOffset, atomLengths[i],
                                 encKey, 5, atomOffset - atomTableOffset));

        break;
      case BookAtomType_XPS:

        atoms[i] =dynamic_cast<BookAtom*>( new LazyBookAtomXPS(randomReader, atomOffset, atomLengths[i],
                                       encKey, 5, atomOffset - atomTableOffset));

        break;
      }
    }
  }
  Vector<BookContent> DeserializeBookContent(DWORD ContentTableLen, BYTE *buf) {
    DWORD parsedContentLen = 0;
    List<BookContent> contnet;
    DWORD ind = 4 /*JUMP over len*/;
    while (parsedContentLen < ContentTableLen - 4) {

      WORD parentIndex = getWORD(buf + ind + parsedContentLen);
      parsedContentLen += 2;
      DWORD groupIndex = getDWORD(buf + ind + parsedContentLen);
      parsedContentLen += 4;
      DWORD atomIndex = getDWORD(buf + ind + parsedContentLen);
      parsedContentLen += 4;
      WORD contentCount = getWORD(buf + ind + parsedContentLen);
      parsedContentLen += 2;
      DWORD groups_byte_index = 0;

      Vector<BookAtomGroup<BookAtom> *> contentGroups(contentCount);
      for (int i = 0; i < contentCount; i++) {
        contentGroups[i] =
            Book::deserialize_group(buf, ind + parsedContentLen, atoms);
        parsedContentLen += getDWORD(buf + ind + parsedContentLen);
      }
      auto pos = BookPosIndicator({(WORD)groupIndex, (WORD)atomIndex});
      auto bC = BookContent(contentGroups, pos, parentIndex);
      contnet.push_back(bC);
    }
    Vector<BookContent> v_content(contnet.begin(), contnet.end());
    return v_content;
  }
  MsdLazyBook(MsdRandomReader *reader)
      : randomReader(reader), Book(BookType::BookType_EPUB) {
    // read first 10 bytes
    auto first10Bytes = reader->read(0, 10);
    if (std::get<0>(first10Bytes) != 10)
      throw "Bad File";
    BYTE *buf = std::get<1>(first10Bytes);
    BYTE fileVersion = *buf;
    if (fileVersion == 0) {
      BYTE btype = *(buf + 1);
      switch (btype) {
      case BookType_EPUB:
        booktype = BookType_EPUB;
        break;
      case BookType_MSDFORMAT:
        booktype = BookType_MSDFORMAT;
        break;
      case BookType_Voice:
        booktype = BookType_Voice;
        break;
      case BookType_PDF:
        booktype = BookType_PDF;
        break;
      }
      DWORD keylen = getDWORD(buf + 2);
      auto keybytes = reader->read(2 + 4, keylen - 4);
      if (std::get<0>(keybytes) != keylen - 4) {
        throw "Bad File Bytes length";
      }
      free(buf);
      encKey = Vector<BYTE>(keylen - 4);
      buf = std::get<1>(keybytes);
      for (int i = 0; i < encKey.size(); i++)
        encKey[i] = buf[i];
      free(buf);
      atomTableOffset = 2 + keylen;

      auto rbytes = reader->read(atomTableOffset, 4);
      if (std::get<0>(rbytes) != 4)
        throw "Bad File";
      buf = std::get<1>(rbytes);
      auto tbSize = getTableSize(buf);
      atomTableLen = tbSize;
      free(buf);
      groupTableOffset = atomTableOffset + tbSize;
      rbytes = reader->read(groupTableOffset, 4);
      if (std::get<0>(rbytes) != 4)
        throw "Bad File Read groupTableOffset";
      buf = std::get<1>(rbytes);
      tbSize = getTableSize(buf);
      groupTableLen = tbSize;
      contentTableOffset = groupTableOffset + tbSize;
      rbytes = reader->read(contentTableOffset, 4);
      if (std::get<0>(rbytes) != 4)
        throw "Bad File Read contentTableOffset";
      buf = std::get<1>(rbytes);
      tbSize = getTableSize(buf);
      contentTableLen = tbSize;
      atomindicesTableOffset = contentTableOffset + tbSize;
      rbytes = reader->read(atomindicesTableOffset, 4);
      if (std::get<0>(rbytes) != 4)
        throw "Bad File Read atomindicesTableOffset";
      buf = std::get<1>(rbytes);
      tbSize = getTableSize(buf);
      atomindicesTableLen = tbSize;

      DWORD atomsCount = (atomindicesTableLen - 4) / 9;

      atomIndices = Vector<DWORD>(atomsCount);
      atomLengths = Vector<DWORD>(atomsCount);
      atomTypes = Vector<BookAtomType>(atomsCount);
      atoms = Vector<BookAtom *>(atomsCount);
      for (int i = 0; i < atomsCount; i++)
        atoms[i] = NULL;
      rbytes = reader->read(atomindicesTableOffset, atomindicesTableLen);
      if (std::get<0>(rbytes) != atomindicesTableLen)
        throw "Bad File Read atomindicesTableLen";
      buf = std::get<1>(rbytes);
      // decrypt:
      auto encKeySize = encKey.size();
      if (encKeySize > 0) // we dont encrypt indices table
        for (int i = 0; i < atomindicesTableLen; i++)
          buf[i] = buf[i] ^ encKey[i % encKeySize];
      DWORD ind = 4;
      for (int i = 0; i < atomsCount; i++) {
        atomIndices[i] = getDWORD(buf + ind);
        ind += 4;
        atomLengths[i] = getDWORD(buf + ind);
        ind += 4;
        BYTE atomtype = *(buf + ind);
        ind += 1;
        atomTypes[i] = (BookAtomType)atomtype;
      }
      free(buf);
      // load atoms
      loadAtoms();

      // load content list
      rbytes = reader->read(contentTableOffset, contentTableLen);
      if (std::get<0>(rbytes) != contentTableLen)
        throw "Bad File Read contentTableLen";
      buf = std::get<1>(rbytes);
      if (encKeySize > 0)
        for (int i = 0; i < contentTableLen; i++)
          buf[i] = buf[i] ^ encKey[i % encKeySize];
      auto contents = DeserializeBookContent(contentTableLen, buf);
      free(buf);
      Contents = contents;

      // load body
      rbytes = reader->read(groupTableOffset, groupTableLen);
      if (std::get<0>(rbytes) != groupTableLen)
        throw "Bad File Read groupTableLen";
      buf = std::get<1>(rbytes);
      if (encKeySize > 0)
        for (int i = 0; i < groupTableLen; i++)
          buf[i] = buf[i] ^ encKey[i % encKeySize];
      DWORD parsedGroup = 0;
      List<BookAtomGroup<BookAtom> *> grps;
      ind = 4;
      while (parsedGroup < groupTableLen - 4) {
        DWORD group_size = getDWORD(buf + ind);
        auto group = Book::deserialize_group(buf, ind, atoms);
        grps.push_back(group);
        ind += group_size;
        parsedGroup += group_size;
      }
      Vector<BookAtomGroup<BookAtom> *> vbg(grps.begin(), grps.end());
      groups = vbg;

      firstAtom = nextAtom(BookPosIndicator());
      lastAtom = BookPosIndicator({0, 0});
      auto groups = decompose();
      int lastgroupindex = groups.size() - 1;
      for (; lastgroupindex > -1; lastgroupindex--) {
        auto atoms = groups[lastgroupindex]->decompose();
        if (atoms.size() == 0) {

          continue;
        } else {
          lastAtom =
              BookPosIndicator({(WORD)lastgroupindex, (WORD)atoms.size() - 1});
          break;
        }
      }
      // calculate total atoms
      totalAtoms = 0;
      for (auto g : groups) {
        totalAtoms += g->decompose().size();
      }

    } else {
      throw "file version is not recognized";
    }
  }
};

#endif