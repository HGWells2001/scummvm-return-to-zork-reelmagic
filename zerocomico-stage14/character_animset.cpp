/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "common/tokenizer.h"

#include "zerocomico-stage14/character_animset.h"

namespace ZeroComico {

namespace {

static Common::String cleanToken(Common::String token) {
	token.trim();
	while (!token.empty() &&
	       (token.lastChar() == ':' || token.lastChar() == ';' ||
	        token.lastChar() == '{' || token.lastChar() == '}')) {
		token.deleteLastChar();
	}
	while (!token.empty() && (token.firstChar() == '"' || token.firstChar() == '\''))
		token.deleteChar(0);
	while (!token.empty() && (token.lastChar() == '"' || token.lastChar() == '\''))
		token.deleteLastChar();
	return token;
}

static Common::String stripComment(const Common::String &line) {
	bool inQuote = false;
	for (uint32 i = 0; i + 1 < line.size(); ++i) {
		if (line[i] == '"')
			inQuote = !inQuote;
		if (!inQuote && line[i] == '/' && line[i + 1] == '/') {
			Common::String result = line.substr(0, i);
			result.trim();
			return result;
		}
	}
	Common::String result = line;
	result.trim();
	return result;
}

static int braceDelta(const Common::String &line) {
	bool inQuote = false;
	int delta = 0;
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

static bool isProvenAnimSetField(const Common::String &key) {
	static const char *const kFields[] = {
		"special_standby", "Attack", "standby", "Blend",
		"backstep", "jump",
		"HitFrontUp", "HitFrontMid", "HitFrontDown",
		"HitRearUp", "HitRearMid", "HitRearDown",
		"HitFrontUpStay", "HitFrontMidStay", "HitFrontDownStay",
		"HitRearUpStay", "HitRearMidStay", "HitRearDownStay",
		"StartAttack", "LoopAttack", "StopAttack",
		"Dief", "Dieb",
		"take_low", "take_mid", "take_high",
		"Attack_Up", "Attack_Mid", "Attack_Down", "Attack_Parata",
		"Combo", "StayComb", "StayToComb", "CombToStay",
		"sword", "turn",
		"TurnSpeed_Stop", "TurnSpeed_Walk", "TurnSpeed_Run", "TurnSpeed_Back",
		"CollisionFront", "CollisionBack",
		"Strafe", "AdditionalStrafe",
		"walk", "JumpBack", "JumpBackFromStayComb",
		"lookat", "HideSubObject", "UnHideSubObject", "Default"
	};

	for (uint32 i = 0; i < ARRAYSIZE(kFields); ++i) {
		if (key.equalsIgnoreCase(kFields[i]))
			return true;
	}
	return false;
}

static void tokenizeLine(const Common::String &line,
                         Common::Array<Common::String> &tokens) {
	tokens.clear();
	Common::StringTokenizer tokenizer(line);
	while (!tokenizer.empty()) {
		Common::String token = cleanToken(tokenizer.nextToken());
		if (!token.empty())
			tokens.push_back(token);
	}
}

} // namespace

const CharacterAnimSetField *CharacterAnimSet::field(
		const Common::String &key) const {
	for (uint32 i = 0; i < fields.size(); ++i) {
		if (fields[i].key.equalsIgnoreCase(key))
			return &fields[i];
	}
	return nullptr;
}

const CharacterAnimSet *CharacterDefinition::animSet(
		const Common::String &name) const {
	for (uint32 i = 0; i < animSets.size(); ++i) {
		if (animSets[i].name.equalsIgnoreCase(name))
			return &animSets[i];
	}
	return nullptr;
}

void CharacterScriptDocument::clear() {
	_characters.clear();
	_startVectors.clear();
}

const CharacterDefinition *CharacterScriptDocument::character(
		const Common::String &name) const {
	for (uint32 i = 0; i < _characters.size(); ++i) {
		if (_characters[i].name.equalsIgnoreCase(name))
			return &_characters[i];
	}
	return nullptr;
}

const CharacterStartVector *CharacterScriptDocument::startVector(
		const Common::String &characterName) const {
	for (uint32 i = 0; i < _startVectors.size(); ++i) {
		if (_startVectors[i].characterName.equalsIgnoreCase(characterName))
			return &_startVectors[i];
	}
	return nullptr;
}

bool CharacterScriptParser::parse(const Common::String &decodedCharScript,
                                  CharacterScriptDocument &out,
                                  Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decodedCharScript.empty()) {
		errorMessage = "Character script is empty";
		return false;
	}

	Common::StringTokenizer lines(decodedCharScript, "\r\n");
	uint32 physicalLine = 0;
	int depth = 0;
	int characterBaseDepth = -1;
	int32 currentCharacter = -1;
	int32 currentAnimSet = -1;

	while (!lines.empty()) {
		Common::String raw = lines.nextToken();
		++physicalLine;
		Common::String code = stripComment(raw);
		if (code.empty())
			continue;

		Common::Array<Common::String> tokens;
		tokenizeLine(code, tokens);

		if (!tokens.empty() && tokens[0].equalsIgnoreCase("SetCharPos_Vector")) {
			if (tokens.size() < 3) {
				errorMessage = Common::String::format(
					"SetCharPos_Vector on line %u is incomplete",
					(uint)physicalLine);
				return false;
			}

			CharacterStartVector position;
			position.lineNumber = physicalLine;
			position.characterName = tokens[1];
			position.helperName = tokens[2];
			out._startVectors.push_back(position);
		}

		if (currentCharacter < 0 &&
		    !tokens.empty() &&
		    tokens[0].equalsIgnoreCase("ge_Character")) {
			if (tokens.size() < 2) {
				errorMessage = Common::String::format(
					"ge_Character on line %u has no name",
					(uint)physicalLine);
				return false;
			}

			CharacterDefinition character;
			character.lineNumber = physicalLine;
			character.name = tokens[1];
			out._characters.push_back(character);
			currentCharacter = (int32)out._characters.size() - 1;
			currentAnimSet = -1;
			characterBaseDepth = depth;
		} else if (currentCharacter >= 0) {
			CharacterDefinition &character = out._characters[currentCharacter];

			if (!tokens.empty() && tokens[0].equalsIgnoreCase("AnimSet")) {
				if (tokens.size() < 3) {
					errorMessage = Common::String::format(
						"AnimSet on line %u is incomplete",
						(uint)physicalLine);
					return false;
				}

				CharacterAnimSet animSet;
				animSet.lineNumber = physicalLine;
				animSet.name = tokens[1];
				animSet.entity = tokens[2];
				character.animSets.push_back(animSet);
				currentAnimSet = (int32)character.animSets.size() - 1;
			} else if (currentAnimSet >= 0 && !tokens.empty() &&
			           isProvenAnimSetField(tokens[0])) {
				CharacterAnimSetField field;
				field.lineNumber = physicalLine;
				field.key = tokens[0];
				field.rawLine = raw;
				for (uint32 i = 1; i < tokens.size(); ++i)
					field.values.push_back(tokens[i]);
				character.animSets[currentAnimSet].fields.push_back(field);
			}
		}

		depth += braceDelta(code);
		if (depth < 0) {
			errorMessage = Common::String::format(
				"Character script closes too many braces on line %u",
				(uint)physicalLine);
			return false;
		}

		if (currentCharacter >= 0 &&
		    characterBaseDepth >= 0 &&
		    depth <= characterBaseDepth &&
		    code.contains("}")) {
			currentCharacter = -1;
			currentAnimSet = -1;
			characterBaseDepth = -1;
		}
	}

	if (depth != 0) {
		errorMessage = "Character script has unbalanced braces";
		return false;
	}

	if (out._characters.empty() && out._startVectors.empty()) {
		errorMessage = "No character declarations or start vectors found";
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
