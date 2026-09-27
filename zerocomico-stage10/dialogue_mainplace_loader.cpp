/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage10/dialogue_mainplace_loader.h"

#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

Common::Path gameplayDialogueScriptPath(const Common::String &mainPlaceName) {
	return Common::Path(Common::String::format("%s/gameplay/dialog.isc",
		normalizeMainPlaceName(mainPlaceName).c_str()));
}

bool DialogueMainPlaceLoader::load(const Common::String &mainPlaceName,
                                   GameplayResourceHost &resources,
                                   DialoguePresentationHost *presentation,
                                   GameplayDialogueService &service,
                                   Common::String &errorMessage) const {
	errorMessage.clear();

	const Common::Path path = gameplayDialogueScriptPath(mainPlaceName);
	Common::String decoded;
	if (!resources.readDecodedText(path, decoded)) {
		errorMessage = Common::String::format("Unable to read %s",
			path.toString().c_str());
		return false;
	}

	if (!service.load(decoded, presentation, errorMessage)) {
		if (errorMessage.empty())
			errorMessage = Common::String::format("Unable to parse %s",
				path.toString().c_str());
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
