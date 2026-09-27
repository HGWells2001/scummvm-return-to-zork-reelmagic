/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE7_GAMEPLAY_MAINPLACE_H
#define ZEROCOMICO_STAGE7_GAMEPLAY_MAINPLACE_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage6/mainplace.h"
#include "zerocomico-stage7/bsp_navigation.h"
#include "zerocomico-stage7/gameplay_object.h"
#include "zerocomico-stage7/gameplay_room.h"

namespace ZeroComico {

class GameplayResourceHost {
public:
	virtual ~GameplayResourceHost() {}

	virtual bool readDecodedText(const Common::Path &path, Common::String &text) = 0;
	virtual bool readPlainText(const Common::Path &path, Common::String &text) = 0;
};

struct GameplayMainPlaceState {
	Common::String mainPlace;
	MainPlaceDescriptor mainPlaceDescriptor;
	GameplayRoomRegistry rooms;
	GameplayRoomDescriptor activeRoom;
	BspNavigation navigation;
	GameplayObjectRegistry objects;

	void clear();
};

class GameplayMainPlaceLoader {
public:
	bool load(const Common::String &mainPlaceName,
	          const MainPlaceDescriptor &mainPlaceDescriptor,
	          GameplayResourceHost &host,
	          GameplayMainPlaceState &out,
	          Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
