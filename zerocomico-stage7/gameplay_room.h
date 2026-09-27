/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE7_GAMEPLAY_ROOM_H
#define ZEROCOMICO_STAGE7_GAMEPLAY_ROOM_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

namespace ZeroComico {

struct GameplayRoomDescriptor {
	Common::String name;
	Common::String prefix;
	Common::String background;
	Common::String camera;
	Common::String navigationMap;
	Common::String cameraMap;
};

class GameplayRoomRegistry {
public:
	void clear();
	bool parse(const Common::String &decodedRoomScript, Common::String &errorMessage);

	int32 find(const Common::String &name) const;
	const GameplayRoomDescriptor *room(int32 id) const;
	uint32 size() const { return _rooms.size(); }

private:
	Common::Array<GameplayRoomDescriptor> _rooms;
};

Common::Path gameplayPuzzleScriptPath(const Common::String &mainPlaceName);
Common::Path gameplayCharacterScriptPath(const Common::String &mainPlaceName);
Common::Path gameplayNavigationPath(const Common::String &mainPlaceName,
                                    const GameplayRoomDescriptor &room);

} // End of namespace ZeroComico

#endif
