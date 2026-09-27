/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE14_CHARACTER_ANIMSET_H
#define ZEROCOMICO_STAGE14_CHARACTER_ANIMSET_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

struct CharacterAnimSetField {
	uint32 lineNumber;
	Common::String key;
	Common::Array<Common::String> values;
	Common::String rawLine;

	CharacterAnimSetField() : lineNumber(0) {}
};

struct CharacterAnimSet {
	uint32 lineNumber;
	Common::String name;
	Common::String entity;
	Common::Array<CharacterAnimSetField> fields;

	CharacterAnimSet() : lineNumber(0) {}

	const CharacterAnimSetField *field(const Common::String &key) const;
};

struct CharacterDefinition {
	uint32 lineNumber;
	Common::String name;
	Common::Array<CharacterAnimSet> animSets;

	CharacterDefinition() : lineNumber(0) {}

	const CharacterAnimSet *animSet(const Common::String &name) const;
};

struct CharacterStartVector {
	uint32 lineNumber;
	Common::String characterName;
	Common::String helperName;

	CharacterStartVector() : lineNumber(0) {}
};

class CharacterScriptDocument {
public:
	void clear();

	const CharacterDefinition *character(const Common::String &name) const;
	const CharacterStartVector *startVector(const Common::String &characterName) const;

	const Common::Array<CharacterDefinition> &characters() const { return _characters; }
	const Common::Array<CharacterStartVector> &startVectors() const { return _startVectors; }

private:
	friend class CharacterScriptParser;
	Common::Array<CharacterDefinition> _characters;
	Common::Array<CharacterStartVector> _startVectors;
};

/**
 * Conservative parser for gameplay/char.isc character bindings.
 *
 * Proven retail forms currently promoted:
 *   ge_Character <name> { ... }
 *   AnimSet <animset-name> <entity>
 *   SetCharPos_Vector <character> <helper>
 *
 * Inside the current AnimSet we preserve only field labels proven by strings
 * in the retail executable, while keeping all values as opaque identifiers.
 */
class CharacterScriptParser {
public:
	bool parse(const Common::String &decodedCharScript,
	           CharacterScriptDocument &out,
	           Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
