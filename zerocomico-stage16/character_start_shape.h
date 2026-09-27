/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE16_CHARACTER_START_SHAPE_H
#define ZEROCOMICO_STAGE16_CHARACTER_START_SHAPE_H

#include "common/str.h"

#include "zerocomico-stage14/character_animset.h"
#include "zerocomico-stage16/shape_document.h"

namespace ZeroComico {

struct CharacterStartShapeResolution {
	Common::String characterName;
	Common::String helperName;
	const CharacterStartVector *startVector;
	const ShapeDefinition *shape;

	CharacterStartShapeResolution() :
		startVector(nullptr),
		shape(nullptr) {}

	bool valid() const {
		return startVector && shape &&
		       shape->kind == kShapeDefinitionPosition;
	}
};

bool resolveCharacterStartShape(const CharacterScriptDocument &characters,
                                const ShapeScriptDocument &shapes,
                                const Common::String &characterName,
                                CharacterStartShapeResolution &out,
                                Common::String &errorMessage);

} // End of namespace ZeroComico

#endif
