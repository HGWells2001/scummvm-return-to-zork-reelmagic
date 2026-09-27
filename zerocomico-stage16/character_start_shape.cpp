/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage16/character_start_shape.h"

namespace ZeroComico {

bool resolveCharacterStartShape(const CharacterScriptDocument &characters,
                                const ShapeScriptDocument &shapes,
                                const Common::String &characterName,
                                CharacterStartShapeResolution &out,
                                Common::String &errorMessage) {
	out = CharacterStartShapeResolution();
	errorMessage.clear();

	const CharacterStartVector *start =
		characters.startVector(characterName);
	if (!start) {
		errorMessage = Common::String::format(
			"Character '%s' has no SetCharPos_Vector binding",
			characterName.c_str());
		return false;
	}

	const ShapeDefinition *shape = shapes.shape(start->helperName);
	if (!shape) {
		errorMessage = Common::String::format(
			"Start helper '%s' for character '%s' is not declared as ge_Shape",
			start->helperName.c_str(), characterName.c_str());
		return false;
	}

	if (shape->kind != kShapeDefinitionPosition) {
		errorMessage = Common::String::format(
			"Start helper '%s' for character '%s' is type %s, not Position",
			shape->name.c_str(), characterName.c_str(),
			shapeDefinitionKindName(shape->kind));
		return false;
	}

	out.characterName = characterName;
	out.helperName = start->helperName;
	out.startVector = start;
	out.shape = shape;
	return true;
}

} // End of namespace ZeroComico
