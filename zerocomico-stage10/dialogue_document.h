/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE10_DIALOGUE_DOCUMENT_H
#define ZEROCOMICO_STAGE10_DIALOGUE_DOCUMENT_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

struct DialogueSpeaker {
	Common::String name;
	Common::String key;
	uint16 red;
	uint16 green;
	uint16 blue;
	float typingSpeed;

	DialogueSpeaker() :
		red(255), green(255), blue(255), typingSpeed(0.0f) {}
};

struct DialogueSpeech {
	Common::String speakerKey;
	Common::String speakerName;
	Common::String text;
};

struct DialogueChoiceSet {
	Common::Array<Common::String> choices;
};

enum DialogueNodeType {
	kDialogueSpeechNode,
	kDialogueChoiceNode
};

struct DialogueNode {
	DialogueNodeType type;
	DialogueSpeech speech;
	DialogueChoiceSet choice;

	DialogueNode() : type(kDialogueSpeechNode) {}
};

struct DialogueBlock {
	Common::String name;
	Common::Array<DialogueNode> nodes;
};

class DialogueDocument {
public:
	void clear();

	const DialogueSpeaker *speakerByName(const Common::String &name) const;
	const DialogueSpeaker *speakerByKey(const Common::String &key) const;
	const DialogueBlock *dialog(const Common::String &name) const;

	const Common::Array<DialogueSpeaker> &speakers() const { return _speakers; }
	const Common::Array<DialogueBlock> &dialogs() const { return _dialogs; }

private:
	friend class DialogueDocumentParser;
	Common::Array<DialogueSpeaker> _speakers;
	Common::Array<DialogueBlock> _dialogs;
};

class DialogueDocumentParser {
public:
	bool parse(const Common::String &decodedDialogScript,
	           DialogueDocument &out,
	           Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
