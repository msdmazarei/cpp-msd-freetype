#include "BookAtomVoice.hpp"

bool BookAtomVoice::is_screen_renderable() { return false; }
bool BookAtomVoice::is_render_decomposable() { return false; };
Vector<wchar_t> BookAtomVoice::decompose() { throw "Not Decomposable"; };

List<BYTE> *BookAtomVoice::serialize_binary() {
  // to Serialize BookAtom Voice:
  // <Total Len> <AtomType [BYTE]> <Duration [DWORD]>
  throw "Not Implemented";
}