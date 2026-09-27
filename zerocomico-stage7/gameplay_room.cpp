/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage7/gameplay_room.h"

#include "common/tokenizer.h"

#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

namespace {

static Common::String cleaned(Common::String value) {
	value.trim();
	while (!value.empty() && (value.lastChar() == ':' || value.lastChar() == '{'))
		value.deleteLastChar();
	while (!value.empty() && (value.firstChar() == '"' || value.firstChar() == '\''))
		value.deleteChar(0);
	while (!value.empty() && (value.lastChar() == '"' || value.lastChar() == '\''))
		value.deleteLastChar();
	return value;
}

static Common::String normalizedKey(Common::String key) {
	key = cleaned(key);
	key.toLowercase();
	return key;
}

static Common::Path ensureBspPath(const Common::String &mainPlaceName,
                                  Common::String resource) {
	resource = cleaned(resource);
	if (resource.empty())
		return Common::Path();

	Common::String lower = resource;
	lower.toLowercase();
	if (!lower.hasSuffix(".bsp"))
		resource += ".bsp";

	if (resource.contains('/'))
		return Common::Path(resource);

	return Common::Path(Common::String::format("%s/gameplay/%s",
		normalizeMainPlaceName(mainPlaceName).c_str(), resource.c_str()));
}

} // namespace

void GameplayRoomRegistry::clear() {
	_rooms.clear();
}

bool GameplayRoomRegistry::parse(const Common::String &decodedRoomScript,
                                 Common::String &errorMessage) {
	clear();
	errorMessage.clear();

	Common::StringTokenizer tokens(decodedRoomScript);
	GameplayRoomDescriptor current;
	bool inRoom = false;
	int depth = 0;

	while (!tokens.empty()) {
		Common::String token = tokens.nextToken();

		if (!inRoom && token.equalsIgnoreCase("Room")) {
			if (tokens.empty())
				break;
			current = GameplayRoomDescriptor();
			current.name = cleaned(tokens.nextToken());
			inRoom = !current.name.empty();
			depth = 0;
			continue;
		}

		if (!inRoom)
			continue;

		if (token == "{") {
			++depth;
			continue;
		}
		if (token == "}") {
			if (depth > 0)
				--depth;
			if (depth == 0) {
				_rooms.push_back(current);
				inRoom = false;
			}
			continue;
		}

		if (depth != 1 || tokens.empty())
			continue;

		const Common::String key = normalizedKey(token);
		if (key == "prefix") {
			current.prefix = cleaned(tokens.nextToken());
		} else if (key == "backgrd" || key == "background") {
			current.background = cleaned(tokens.nextToken());
		} else if (key == "camera") {
			current.camera = cleaned(tokens.nextToken());
		} else if (key == "map" || key == "bsp" || key == "navigationmap") {
			current.navigationMap = cleaned(tokens.nextToken());
		} else if (key == "mapcam" || key == "cameramap") {
			current.cameraMap = cleaned(tokens.nextToken());
		}
	}

	if (inRoom && !current.name.empty())
		_rooms.push_back(current);

	if (_rooms.empty()) {
		errorMessage = "No Room declarations found";
		return false;
	}
	return true;
}

int32 GameplayRoomRegistry::find(const Common::String &name) const {
	for (uint32 i = 0; i < _rooms.size(); ++i) {
		if (_rooms[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

const GameplayRoomDescriptor *GameplayRoomRegistry::room(int32 id) const {
	if (id < 0 || (uint32)id >= _rooms.size())
		return nullptr;
	return &_rooms[id];
}

Common::Path gameplayPuzzleScriptPath(const Common::String &mainPlaceName) {
	return Common::Path(Common::String::format("%s/gameplay/puzzle.isc",
		normalizeMainPlaceName(mainPlaceName).c_str()));
}

Common::Path gameplayCharacterScriptPath(const Common::String &mainPlaceName) {
	return Common::Path(Common::String::format("%s/gameplay/char.isc",
		normalizeMainPlaceName(mainPlaceName).c_str()));
}

Common::Path gameplayNavigationPath(const Common::String &mainPlaceName,
                                    const GameplayRoomDescriptor &room) {
	if (!room.navigationMap.empty())
		return ensureBspPath(mainPlaceName, room.navigationMap);

	if (!room.prefix.empty())
		return ensureBspPath(mainPlaceName, room.prefix + "Map00");

	return Common::Path();
}

} // End of namespace ZeroComico
