/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage16/character_start_binding.h"

#include "zerocomico-stage6/mainplace.h"
#include "zerocomico-stage7/gameplay_room.h"

namespace ZeroComico {

Common::Path gameplayShapeScriptPath(const Common::String &mainPlaceName) {
	return Common::Path(Common::String::format("%s/gameplay/Shape.shp",
		normalizeMainPlaceName(mainPlaceName).c_str()));
}

bool CharacterStartBindingLoader::load(
		const Common::String &mainPlaceName,
		const Common::String &characterName,
		GameplayResourceHost &resources,
		CharacterStartBinding &out,
		Common::String &errorMessage) const {
	errorMessage.clear();
	CharacterStartBinding prepared;

	Common::String charText;
	const Common::Path charPath = gameplayCharacterScriptPath(mainPlaceName);
	if (!resources.readDecodedText(charPath, charText)) {
		errorMessage = Common::String::format(
			"Unable to read %s", charPath.toString().c_str());
		return false;
	}

	CharacterScriptParser charParser;
	if (!charParser.parse(charText, prepared.characters, errorMessage))
		return false;

	Common::String shapeText;
	const Common::Path shapePath = gameplayShapeScriptPath(mainPlaceName);
	if (!resources.readDecodedText(shapePath, shapeText)) {
		errorMessage = Common::String::format(
			"Unable to read %s", shapePath.toString().c_str());
		return false;
	}

	ShapeScriptParser shapeParser;
	if (!shapeParser.parse(shapeText, prepared.shapes, errorMessage))
		return false;

	if (!resolveCharacterStartShape(
			prepared.characters, prepared.shapes, characterName,
			prepared.resolution, errorMessage))
		return false;

	out = prepared;
	return true;
}

} // End of namespace ZeroComico
