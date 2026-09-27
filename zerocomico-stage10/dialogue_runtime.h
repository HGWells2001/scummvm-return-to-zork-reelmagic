/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE10_DIALOGUE_RUNTIME_H
#define ZEROCOMICO_STAGE10_DIALOGUE_RUNTIME_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage10/dialogue_document.h"

namespace ZeroComico {

class DialoguePresentationHost {
public:
	virtual ~DialoguePresentationHost() {}

	virtual void showSpeech(const DialogueSpeaker &speaker,
	                        const Common::String &text) = 0;
	virtual void hideSpeech() = 0;

	virtual void showChoices(const Common::Array<Common::String> &choices) = 0;
	virtual void hideChoices() = 0;
};

enum DialogueRuntimeState {
	kDialogueStopped,
	kDialogueShowingSpeech,
	kDialogueWaitingChoice,
	kDialogueChoiceBranchUnresolved,
	kDialogueFinished
};

class DialogueRuntime {
public:
	DialogueRuntime();

	void bind(const DialogueDocument *document, DialoguePresentationHost *host);
	void clear();

	bool start(const Common::String &initiator,
	           const Common::String &dialogName);
	bool advanceSpeech();
	bool selectChoice(uint32 choiceIndex);

	bool isPlaying() const;
	DialogueRuntimeState state() const { return _state; }

	const Common::String &initiator() const { return _initiator; }
	const Common::String &dialogName() const { return _dialogName; }
	int32 selectedChoice() const { return _selectedChoice; }
	uint32 nodeIndex() const { return _nodeIndex; }

private:
	bool presentCurrent();
	void finish();

	const DialogueDocument *_document;
	DialoguePresentationHost *_host;
	const DialogueBlock *_dialog;
	Common::String _initiator;
	Common::String _dialogName;
	uint32 _nodeIndex;
	int32 _selectedChoice;
	DialogueRuntimeState _state;
};

} // End of namespace ZeroComico

#endif
