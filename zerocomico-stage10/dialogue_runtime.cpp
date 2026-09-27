/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage10/dialogue_runtime.h"

namespace ZeroComico {

DialogueRuntime::DialogueRuntime() :
	_document(nullptr),
	_host(nullptr),
	_dialog(nullptr),
	_nodeIndex(0),
	_selectedChoice(-1),
	_state(kDialogueStopped) {
}

void DialogueRuntime::bind(const DialogueDocument *document,
                           DialoguePresentationHost *host) {
	clear();
	_document = document;
	_host = host;
}

void DialogueRuntime::clear() {
	if (_host) {
		_host->hideSpeech();
		_host->hideChoices();
	}

	_dialog = nullptr;
	_initiator.clear();
	_dialogName.clear();
	_nodeIndex = 0;
	_selectedChoice = -1;
	_state = kDialogueStopped;
}

bool DialogueRuntime::start(const Common::String &initiator,
                            const Common::String &dialogName) {
	if (!_document || !_host)
		return false;

	const DialogueBlock *dialog = _document->dialog(dialogName);
	if (!dialog)
		return false;

	_host->hideSpeech();
	_host->hideChoices();
	_dialog = dialog;
	_initiator = initiator;
	_dialogName = dialogName;
	_nodeIndex = 0;
	_selectedChoice = -1;

	if (_dialog->nodes.empty()) {
		finish();
		return true;
	}
	return presentCurrent();
}

bool DialogueRuntime::presentCurrent() {
	if (!_dialog || !_host)
		return false;

	if (_nodeIndex >= _dialog->nodes.size()) {
		finish();
		return true;
	}

	const DialogueNode &node = _dialog->nodes[_nodeIndex];
	if (node.type == kDialogueSpeechNode) {
		const DialogueSpeaker *speaker =
			_document->speakerByKey(node.speech.speakerKey);
		if (!speaker)
			return false;

		_host->hideChoices();
		_host->showSpeech(*speaker, node.speech.text);
		_state = kDialogueShowingSpeech;
		return true;
	}

	_host->hideSpeech();
	_host->showChoices(node.choice.choices);
	_state = kDialogueWaitingChoice;
	return true;
}

bool DialogueRuntime::advanceSpeech() {
	if (_state != kDialogueShowingSpeech || !_dialog || !_host)
		return false;

	_host->hideSpeech();
	++_nodeIndex;
	return presentCurrent();
}

bool DialogueRuntime::selectChoice(uint32 choiceIndex) {
	if (_state != kDialogueWaitingChoice || !_dialog || !_host ||
	    _nodeIndex >= _dialog->nodes.size())
		return false;

	const DialogueNode &node = _dialog->nodes[_nodeIndex];
	if (node.type != kDialogueChoiceNode ||
	    choiceIndex >= node.choice.choices.size())
		return false;

	_selectedChoice = (int32)choiceIndex;
	_host->hideChoices();

	// The public corpus establishes the choice text, but not the retail
	// branch-dispatch semantics inside BEGIN/END blocks. Do not guess a jump.
	// Keep wait_last_dialog active until a future Sequence/Dialog branch
	// decoder explicitly resumes the selected branch.
	_state = kDialogueChoiceBranchUnresolved;
	return true;
}

bool DialogueRuntime::isPlaying() const {
	return _state == kDialogueShowingSpeech ||
	       _state == kDialogueWaitingChoice ||
	       _state == kDialogueChoiceBranchUnresolved;
}

void DialogueRuntime::finish() {
	if (_host) {
		_host->hideSpeech();
		_host->hideChoices();
	}
	_state = kDialogueFinished;
	_dialog = nullptr;
}

} // End of namespace ZeroComico
