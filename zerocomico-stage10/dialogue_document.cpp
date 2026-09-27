/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "common/tokenizer.h"

#include "zerocomico-stage10/dialogue_document.h"

namespace ZeroComico {

namespace {

static Common::String cleanToken(Common::String value) {
	value.trim();
	while (!value.empty() &&
	       (value.lastChar() == ':' || value.lastChar() == '{' ||
	        value.lastChar() == '}' || value.lastChar() == ';'))
		value.deleteLastChar();
	while (!value.empty() && (value.firstChar() == '"' || value.firstChar() == '\''))
		value.deleteChar(0);
	while (!value.empty() && (value.lastChar() == '"' || value.lastChar() == '\''))
		value.deleteLastChar();
	return value;
}

static bool parseUnsigned(const Common::String &value, uint16 &out) {
	if (value.empty())
		return false;

	uint32 n = 0;
	for (uint32 i = 0; i < value.size(); ++i) {
		const char c = value[i];
		if (c < '0' || c > '9')
			return false;
		n = n * 10 + (uint32)(c - '0');
		if (n > 65535)
			return false;
	}
	out = (uint16)n;
	return true;
}

static bool parseFloatSimple(const Common::String &value, float &out) {
	if (value.empty())
		return false;

	bool negative = false;
	uint32 i = 0;
	if (value[0] == '-') {
		negative = true;
		i = 1;
	}
	if (i >= value.size())
		return false;

	float whole = 0.0f;
	float fraction = 0.0f;
	float divisor = 1.0f;
	bool afterDot = false;
	bool any = false;

	for (; i < value.size(); ++i) {
		const char c = value[i];
		if (c == '.' && !afterDot) {
			afterDot = true;
			continue;
		}
		if (c < '0' || c > '9')
			return false;
		any = true;
		if (!afterDot) {
			whole = whole * 10.0f + (float)(c - '0');
		} else {
			divisor *= 10.0f;
			fraction += (float)(c - '0') / divisor;
		}
	}
	if (!any)
		return false;

	out = whole + fraction;
	if (negative)
		out = -out;
	return true;
}

static Common::Array<Common::String> quotedStrings(const Common::String &line) {
	Common::Array<Common::String> out;
	bool inQuote = false;
	Common::String current;

	for (uint32 i = 0; i < line.size(); ++i) {
		const char c = line[i];
		if (c == '"') {
			if (inQuote) {
				out.push_back(current);
				current.clear();
			}
			inQuote = !inQuote;
			continue;
		}
		if (inQuote)
			current += c;
	}
	return out;
}

static int braceDelta(const Common::String &line) {
	int delta = 0;
	bool inQuote = false;
	for (uint32 i = 0; i < line.size(); ++i) {
		const char c = line[i];
		if (c == '"') {
			inQuote = !inQuote;
			continue;
		}
		if (inQuote)
			continue;
		if (c == '{')
			++delta;
		else if (c == '}')
			--delta;
	}
	return delta;
}

static bool parseSpeakerLine(const Common::String &line, DialogueSpeaker &speaker) {
	Common::StringTokenizer tokens(line);
	if (tokens.empty() || !tokens.nextToken().equalsIgnoreCase("speaker"))
		return false;

	Common::Array<Common::String> fields;
	while (!tokens.empty())
		fields.push_back(cleanToken(tokens.nextToken()));
	if (fields.size() != 6)
		return false;

	speaker = DialogueSpeaker();
	speaker.name = fields[0];
	speaker.key = fields[1];
	if (!parseUnsigned(fields[2], speaker.red) ||
	    !parseUnsigned(fields[3], speaker.green) ||
	    !parseUnsigned(fields[4], speaker.blue) ||
	    !parseFloatSimple(fields[5], speaker.typingSpeed))
		return false;

	return !speaker.name.empty() && speaker.key.size() == 1;
}

static bool parseSpeechLine(const Common::String &line,
                            const DialogueDocument &document,
                            DialogueSpeech &speech) {
	Common::String trimmed = line;
	trimmed.trim();
	if (trimmed.empty())
		return false;

	Common::StringTokenizer tokens(trimmed);
	if (tokens.empty())
		return false;
	const Common::String key = cleanToken(tokens.nextToken());
	if (key.size() != 1)
		return false;

	const Common::Array<Common::String> quoted = quotedStrings(trimmed);
	if (quoted.size() != 1)
		return false;

	const DialogueSpeaker *speaker = document.speakerByKey(key);
	if (!speaker)
		return false;

	speech.speakerKey = key;
	speech.speakerName = speaker->name;
	speech.text = quoted[0];
	return true;
}

} // namespace

void DialogueDocument::clear() {
	_speakers.clear();
	_dialogs.clear();
}

const DialogueSpeaker *DialogueDocument::speakerByName(const Common::String &name) const {
	for (uint32 i = 0; i < _speakers.size(); ++i) {
		if (_speakers[i].name.equalsIgnoreCase(name))
			return &_speakers[i];
	}
	return nullptr;
}

const DialogueSpeaker *DialogueDocument::speakerByKey(const Common::String &key) const {
	for (uint32 i = 0; i < _speakers.size(); ++i) {
		if (_speakers[i].key.equalsIgnoreCase(key))
			return &_speakers[i];
	}
	return nullptr;
}

const DialogueBlock *DialogueDocument::dialog(const Common::String &name) const {
	for (uint32 i = 0; i < _dialogs.size(); ++i) {
		if (_dialogs[i].name.equalsIgnoreCase(name))
			return &_dialogs[i];
	}
	return nullptr;
}

bool DialogueDocumentParser::parse(const Common::String &decodedDialogScript,
                                   DialogueDocument &out,
                                   Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	Common::StringTokenizer lines(decodedDialogScript, "\r\n");
	DialogueBlock currentDialog;
	bool inDialog = false;
	int dialogDepth = 0;
	bool inChoice = false;
	DialogueChoiceSet currentChoice;

	while (!lines.empty()) {
		Common::String line = lines.nextToken();
		line.trim();
		if (line.empty() || line.hasPrefix("//"))
			continue;

		if (!inDialog) {
			DialogueSpeaker speaker;
			if (parseSpeakerLine(line, speaker)) {
				out._speakers.push_back(speaker);
				continue;
			}

			Common::StringTokenizer tokens(line);
			if (tokens.empty())
				continue;
			const Common::String first = cleanToken(tokens.nextToken());
			if (!first.equalsIgnoreCase("Dialog"))
				continue;
			if (tokens.empty()) {
				errorMessage = "Dialog declaration without a name";
				return false;
			}

			currentDialog = DialogueBlock();
			currentDialog.name = cleanToken(tokens.nextToken());
			if (currentDialog.name.empty()) {
				errorMessage = "Empty Dialog name";
				return false;
			}
			inDialog = true;
			dialogDepth = braceDelta(line);
			if (dialogDepth < 0) {
				errorMessage = "Invalid Dialog brace structure";
				return false;
			}
			continue;
		}

		dialogDepth += braceDelta(line);
		if (dialogDepth < 0) {
			errorMessage = Common::String::format(
				"Dialog '%s' closes too many braces", currentDialog.name.c_str());
			return false;
		}

		Common::StringTokenizer tokens(line);
		Common::String first;
		if (!tokens.empty())
			first = cleanToken(tokens.nextToken());

		if (first.equalsIgnoreCase("BEGIN")) {
			if (inChoice) {
				errorMessage = "Nested BEGIN choice block is unsupported";
				return false;
			}
			inChoice = true;
			currentChoice = DialogueChoiceSet();
			continue;
		}

		if (first.equalsIgnoreCase("END") && inChoice) {
			DialogueNode node;
			node.type = kDialogueChoiceNode;
			node.choice = currentChoice;
			currentDialog.nodes.push_back(node);
			inChoice = false;
			currentChoice = DialogueChoiceSet();
			continue;
		}

		if (inChoice) {
			const Common::Array<Common::String> quoted = quotedStrings(line);
			for (uint32 i = 0; i < quoted.size(); ++i)
				currentChoice.choices.push_back(quoted[i]);
		} else {
			DialogueSpeech speech;
			if (parseSpeechLine(line, out, speech)) {
				DialogueNode node;
				node.type = kDialogueSpeechNode;
				node.speech = speech;
				currentDialog.nodes.push_back(node);
			}
		}

		if (dialogDepth == 0) {
			if (inChoice) {
				errorMessage = Common::String::format(
					"Dialog '%s' ends inside a choice block", currentDialog.name.c_str());
				return false;
			}
			out._dialogs.push_back(currentDialog);
			currentDialog = DialogueBlock();
			inDialog = false;
		}
	}

	if (inDialog) {
		errorMessage = Common::String::format(
			"Dialog '%s' did not close", currentDialog.name.c_str());
		return false;
	}

	if (out._speakers.empty()) {
		errorMessage = "No speaker table found";
		return false;
	}
	if (out._dialogs.empty()) {
		errorMessage = "No Dialog blocks found";
		return false;
	}
	return true;
}

} // End of namespace ZeroComico
