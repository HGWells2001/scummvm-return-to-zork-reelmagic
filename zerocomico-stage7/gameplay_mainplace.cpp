/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage7/gameplay_mainplace.h"

namespace ZeroComico {

void GameplayMainPlaceState::clear() {
	mainPlace.clear();
	mainPlaceDescriptor = MainPlaceDescriptor();
	rooms.clear();
	activeRoom = GameplayRoomDescriptor();
	navigation.clear();
	objects.clear();
}

bool GameplayMainPlaceLoader::load(const Common::String &mainPlaceName,
                                   const MainPlaceDescriptor &mainPlaceDescriptor,
                                   GameplayResourceHost &host,
                                   GameplayMainPlaceState &out,
                                   Common::String &errorMessage) const {
	GameplayMainPlaceState prepared;
	prepared.mainPlace = normalizeMainPlaceName(mainPlaceName);
	prepared.mainPlaceDescriptor = mainPlaceDescriptor;
	errorMessage.clear();

	Common::String roomScript;
	const Common::Path roomPath = mainPlaceRoomScriptPath(prepared.mainPlace);
	if (!host.readDecodedText(roomPath, roomScript)) {
		errorMessage = Common::String::format("Unable to read %s",
			roomPath.toString().c_str());
		return false;
	}

	if (!prepared.rooms.parse(roomScript, errorMessage))
		return false;

	const int32 roomId = prepared.rooms.find(mainPlaceDescriptor.startPlace);
	const GameplayRoomDescriptor *room = prepared.rooms.room(roomId);
	if (!room) {
		errorMessage = Common::String::format("StartPlace '%s' is not a declared Room",
			mainPlaceDescriptor.startPlace.c_str());
		return false;
	}
	prepared.activeRoom = *room;

	const Common::Path navPath = gameplayNavigationPath(prepared.mainPlace,
		prepared.activeRoom);
	if (navPath.empty()) {
		errorMessage = Common::String::format("Room '%s' has no navigation resource",
			prepared.activeRoom.name.c_str());
		return false;
	}

	Common::String bspText;
	if (!host.readPlainText(navPath, bspText)) {
		errorMessage = Common::String::format("Unable to read %s",
			navPath.toString().c_str());
		return false;
	}
	if (!prepared.navigation.parse(bspText, errorMessage))
		return false;

	Common::String puzzleScript;
	const Common::Path puzzlePath = gameplayPuzzleScriptPath(prepared.mainPlace);
	if (!host.readDecodedText(puzzlePath, puzzleScript)) {
		errorMessage = Common::String::format("Unable to read %s",
			puzzlePath.toString().c_str());
		return false;
	}
	if (!prepared.objects.parse(puzzleScript, errorMessage))
		return false;

	out = prepared;
	return true;
}

} // End of namespace ZeroComico
