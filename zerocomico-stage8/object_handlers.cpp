/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage8/object_handlers.h"

#include "common/tokenizer.h"

namespace ZeroComico {

namespace {

static Common::String firstToken(Common::String line) {
	line.trim();
	Common::StringTokenizer tokens(line);
	if (tokens.empty())
		return Common::String();

	Common::String token = tokens.nextToken();
	while (!token.empty() && (token.lastChar() == ':' || token.lastChar() == '{'))
		token.deleteLastChar();
	return token;
}

static Common::String secondToken(Common::String line) {
	line.trim();
	Common::StringTokenizer tokens(line);
	if (tokens.empty())
		return Common::String();
	tokens.nextToken();
	if (tokens.empty())
		return Common::String();

	Common::String token = tokens.nextToken();
	while (!token.empty() && (token.lastChar() == ':' || token.lastChar() == '{'))
		token.deleteLastChar();
	return token;
}

static bool isMetadataLine(const Common::String &token) {
	static const char *const kMetadata[] = {
		"PICKABLE", "EXAMINABLE", "OPERATED", "EXAMINATED",
		"AUTOCAMERA", "RANDOMPOS", "COMBINED", "ASSIGNED",
		"ENABLED", "INSIDE", "COLLISION", "SOUNDSTATE",
		"entity", "polygon", "range", "oprange", "size",
		"roomscope", "examine_text", "take_none", "take_low",
		"take_mid", "take_high"
	};

	for (uint32 i = 0; i < ARRAYSIZE(kMetadata); ++i) {
		if (token.equalsIgnoreCase(kMetadata[i]))
			return true;
	}
	return false;
}

} // namespace

void ObjectHandlerRegistry::clear() {
	_handlers.clear();
}

void ObjectHandlerRegistry::parse(const Common::String &decodedPuzzleScript) {
	clear();

	Common::String currentObject;
	int32 activeHandler = -1;

	Common::StringTokenizer lines(decodedPuzzleScript, "\r\n");
	while (!lines.empty()) {
		Common::String line = lines.nextToken();
		line.trim();
		if (line.empty() || line.hasPrefix("//"))
			continue;

		const Common::String token = firstToken(line);
		if (token.equalsIgnoreCase("Object")) {
			currentObject = secondToken(line);
			activeHandler = -1;
			continue;
		}

		if (currentObject.empty())
			continue;

		if (token.equalsIgnoreCase("examine") || token.equalsIgnoreCase("operate")) {
			ObjectHandlerBody body;
			body.objectName = currentObject;
			body.verb = token.equalsIgnoreCase("examine")
				? kGameplayExamine : kGameplayOperate;
			_handlers.push_back(body);
			activeHandler = (int32)_handlers.size() - 1;
			continue;
		}

		if (isMetadataLine(token)) {
			activeHandler = -1;
			continue;
		}

		if (activeHandler >= 0)
			_handlers[activeHandler].lines.push_back(line);
	}
}

const ObjectHandlerBody *ObjectHandlerRegistry::find(
		const Common::String &objectName, GameplayVerb verb) const {
	for (uint32 i = 0; i < _handlers.size(); ++i) {
		if (_handlers[i].verb == verb &&
		    _handlers[i].objectName.equalsIgnoreCase(objectName))
			return &_handlers[i];
	}
	return nullptr;
}

} // End of namespace ZeroComico
