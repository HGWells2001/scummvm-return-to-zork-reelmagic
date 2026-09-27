/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE10_GAMEPLAY_DIALOGUE_SERVICE_H
#define ZEROCOMICO_STAGE10_GAMEPLAY_DIALOGUE_SERVICE_H

#include "common/str.h"

#include "zerocomico-stage10/dialogue_document.h"
#include "zerocomico-stage10/dialogue_runtime.h"

namespace ZeroComico {

/**
 * Small service intended to back Stage 8 GameplayRuntimeHost::startDialog()
 * and ::isDialogPlaying().
 */
class GameplayDialogueService {
public:
	GameplayDialogueService();

	bool load(const Common::String &decodedDialogScript,
	          DialoguePresentationHost *presentation,
	          Common::String &errorMessage);

	bool startDialog(const Common::String &speaker,
	                 const Common::String &dialogName);
	bool isDialogPlaying() const;

	bool advanceSpeech();
	bool selectChoice(uint32 choiceIndex);

	const DialogueDocument &document() const { return _document; }
	const DialogueRuntime &runtime() const { return _runtime; }

private:
	DialogueDocument _document;
	DialogueRuntime _runtime;
};

} // End of namespace ZeroComico

#endif
