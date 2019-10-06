#include "BookAtom.hpp"

BookAtom::BookAtom(BookAtomType atom_type) : atom_type(atom_type) {}
ClassName BookAtom::getType() { return ClassName_BookAtom; }

