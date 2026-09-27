/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE16_CHARACTER_START_BINDING_H
#define ZEROCOMICO_STAGE16_CHARACTER_START_BINDING_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage14/character_animset.h"
#include "zerocomico-stage16/character_start_shape.h"
#include "zerocomico-stage16/shape_document.h"

namespace ZeroComico {

struct CharacterStartBinding {
	CharacterScriptDocument characters;
	ShapeScriptDocument shapes;
	CharacterStartShapeResolution resolution;

	void clear() {
		characters.clear();
		shapes.clear();
		resolution = CharacterStartShapeResolution();
	}

	bool valid() const {
		return resolution.valid();
	}
};

Common::Path gameplayShapeScriptPath(const Common::String &mainPlaceName);

/**
 * Loads the retail character/shape scripts and resolves
 * SetCharPos_Vector -> ge_Shape ... Position transactionally.
 *
 * It does not assign coordinates to the Position shape yet.
 */
class CharacterStartBindingLoader {
public:
	bool load(const Common::String &mainPlaceName,
	          const Common::String &characterName,
	          GameplayResourceHost &resources,
	          CharacterStartBinding &out,
	          Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
