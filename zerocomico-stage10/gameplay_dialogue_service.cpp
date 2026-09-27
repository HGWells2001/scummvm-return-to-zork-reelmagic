/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage10/gameplay_dialogue_service.h"

namespace ZeroComico {

GameplayDialogueService::GameplayDialogueService() {
}

bool GameplayDialogueService::load(const Common::String &decodedDialogScript,
                                   DialoguePresentationHost *presentation,
                                   Common::String &errorMessage) {
	errorMessage.clear();

	DialogueDocument parsed;
	DialogueDocumentParser parser;
	if (!parser.parse(decodedDialogScript, parsed, errorMessage))
		return false;

	_document = parsed;
	_runtime.bind(&_document, presentation);
	return true;
}

bool GameplayDialogueService::startDialog(const Common::String &speaker,
                                          const Common::String &dialogName) {
	return _runtime.start(speaker, dialogName);
}

bool GameplayDialogueService::isDialogPlaying() const {
	return _runtime.isPlaying();
}

bool GameplayDialogueService::advanceSpeech() {
	return _runtime.advanceSpeech();
}

bool GameplayDialogueService::selectChoice(uint32 choiceIndex) {
	return _runtime.selectChoice(choiceIndex);
}

} // End of namespace ZeroComico
