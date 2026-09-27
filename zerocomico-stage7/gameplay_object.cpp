/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage7/gameplay_object.h"

#include "common/tokenizer.h"

#include <cstdio>

namespace ZeroComico {

namespace {

static Common::String normalizeToken(Common::String value) {
	value.trim();
	while (!value.empty() && (value.lastChar() == ':' || value.lastChar() == '{'))
		value.deleteLastChar();
	while (!value.empty() && (value.firstChar() == '"' || value.firstChar() == '\''))
		value.deleteChar(0);
	while (!value.empty() && (value.lastChar() == '"' || value.lastChar() == '\''))
		value.deleteLastChar();
	return value;
}

static bool parseFloatToken(const Common::String &value, float &out) {
	return std::sscanf(value.c_str(), "%f", &out) == 1;
}

static bool parseFlagValue(const Common::String &value) {
	Common::String lower = normalizeToken(value);
	lower.toLowercase();
	return lower == "1" || lower == "true" || lower == "yes" || lower == "on";
}

static Common::String valueAfterFirstToken(const Common::String &line) {
	Common::StringTokenizer tok(line);
	if (tok.empty())
		return Common::String();
	tok.nextToken();

	Common::String out;
	while (!tok.empty()) {
		if (!out.empty())
			out += " ";
		out += tok.nextToken();
	}
	return normalizeToken(out);
}

static bool firstTokenEquals(const Common::String &line, const char *name,
                             Common::String &value) {
	Common::StringTokenizer tok(line);
	if (tok.empty())
		return false;

	Common::String first = normalizeToken(tok.nextToken());
	if (!first.equalsIgnoreCase(name))
		return false;

	value.clear();
	if (!tok.empty())
		value = normalizeToken(tok.nextToken());
	return true;
}

} // namespace

GameplayObject::GameplayObject() :
	range(0.0f),
	operateRange(0.0f),
	size(0.0f),
	pickable(false),
	examinable(false),
	operated(false),
	enabled(true),
	hasExamineHandler(false),
	hasOperateHandler(false) {
}

void GameplayObjectRegistry::clear() {
	_objects.clear();
}

bool GameplayObjectRegistry::parse(const Common::String &decodedPuzzleScript,
                                   Common::String &errorMessage) {
	clear();
	errorMessage.clear();

	GameplayObject current;
	bool haveCurrent = false;

	Common::StringTokenizer lines(decodedPuzzleScript, "\r\n");
	while (!lines.empty()) {
		Common::String line = lines.nextToken();
		line.trim();
		if (line.empty() || line.hasPrefix("//"))
			continue;

		Common::String value;
		if (firstTokenEquals(line, "Object", value)) {
			if (haveCurrent && !current.name.empty())
				_objects.push_back(current);
			current = GameplayObject();
			current.name = value;
			haveCurrent = !current.name.empty();
			continue;
		}

		if (!haveCurrent)
			continue;

		if (firstTokenEquals(line, "entity", value)) {
			current.entity = value;
		} else if (firstTokenEquals(line, "polygon", value)) {
			current.polygon = value;
		} else if (firstTokenEquals(line, "roomscope", value)) {
			current.roomScope = value;
		} else if (firstTokenEquals(line, "range", value)) {
			parseFloatToken(value, current.range);
		} else if (firstTokenEquals(line, "oprange", value)) {
			parseFloatToken(value, current.operateRange);
		} else if (firstTokenEquals(line, "size", value)) {
			parseFloatToken(value, current.size);
		} else if (firstTokenEquals(line, "examine_text", value)) {
			current.examineText = valueAfterFirstToken(line);
		} else if (firstTokenEquals(line, "PICKABLE", value)) {
			current.pickable = parseFlagValue(value);
		} else if (firstTokenEquals(line, "EXAMINABLE", value)) {
			current.examinable = parseFlagValue(value);
		} else if (firstTokenEquals(line, "OPERATED", value)) {
			current.operated = parseFlagValue(value);
		} else if (firstTokenEquals(line, "ENABLED", value)) {
			current.enabled = parseFlagValue(value);
		} else {
			Common::String first;
			if (firstTokenEquals(line, "examine", first))
				current.hasExamineHandler = true;
			else if (firstTokenEquals(line, "operate", first))
				current.hasOperateHandler = true;
		}
	}

	if (haveCurrent && !current.name.empty())
		_objects.push_back(current);

	if (_objects.empty()) {
		errorMessage = "No Object declarations found in puzzle script";
		return false;
	}

	return true;
}

int32 GameplayObjectRegistry::findByName(const Common::String &name) const {
	for (uint32 i = 0; i < _objects.size(); ++i) {
		if (_objects[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

int32 GameplayObjectRegistry::findByEntity(const Common::String &entity) const {
	for (uint32 i = 0; i < _objects.size(); ++i) {
		if (!_objects[i].entity.empty() && _objects[i].entity.equalsIgnoreCase(entity))
			return (int32)i;
	}
	return -1;
}

int32 GameplayObjectRegistry::resolvePickedEntity(const Common::String &sceneEntity) const {
	int32 id = findByEntity(sceneEntity);
	if (id < 0)
		id = findByName(sceneEntity);
	return id;
}

const GameplayObject *GameplayObjectRegistry::object(int32 id) const {
	if (id < 0 || (uint32)id >= _objects.size())
		return nullptr;
	return &_objects[id];
}

} // End of namespace ZeroComico
