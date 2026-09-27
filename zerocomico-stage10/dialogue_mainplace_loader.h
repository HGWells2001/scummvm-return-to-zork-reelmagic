/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE10_DIALOGUE_MAINPLACE_LOADER_H
#define ZEROCOMICO_STAGE10_DIALOGUE_MAINPLACE_LOADER_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage10/gameplay_dialogue_service.h"

namespace ZeroComico {

Common::Path gameplayDialogueScriptPath(const Common::String &mainPlaceName);

class DialogueMainPlaceLoader {
public:
	bool load(const Common::String &mainPlaceName,
	          GameplayResourceHost &resources,
	          DialoguePresentationHost *presentation,
	          GameplayDialogueService &service,
	          Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
