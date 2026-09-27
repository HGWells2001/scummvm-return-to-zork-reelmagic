/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage20/character_start_placement.h"

namespace ZeroComico {

bool CharacterStartPlacementLoader::load(
		const Common::String &mainPlaceName,
		const Common::String &characterName,
		GameplayResourceHost &resources,
		CharacterStartPlacement &out,
		Common::String &errorMessage) const {
	errorMessage.clear();
	CharacterStartPlacement prepared;

	CharacterStartBindingLoader bindingLoader;
	if (!bindingLoader.load(mainPlaceName, characterName, resources,
	                        prepared.binding, errorMessage))
		return false;

	Common::String shapeText;
	const Common::Path shapePath = gameplayShapeScriptPath(mainPlaceName);
	if (!resources.readDecodedText(shapePath, shapeText)) {
		errorMessage = Common::String::format(
			"Unable to read %s for numeric shape geometry",
			shapePath.toString().c_str());
		return false;
	}

	ShapeGeometryParser geometryParser;
	if (!geometryParser.parse(shapeText, prepared.geometry, errorMessage))
		return false;

	if (!resolveCharacterStartGeometry(
			characterName,
			prepared.binding.resolution.helperName,
			prepared.geometry,
			prepared.resolvedGeometry,
			errorMessage))
		return false;

	out = prepared;
	return true;
}

} // End of namespace ZeroComico
