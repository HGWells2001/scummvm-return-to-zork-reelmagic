/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage23/gameplay_room_transition_host.h"

namespace ZeroComico {

namespace {

static bool sameWhenBothPresent(const Common::String &a,
                                const Common::String &b) {
	return a.empty() || b.empty() || a.equalsIgnoreCase(b);
}

} // namespace

GameplayRoomTransitionHost::GameplayRoomTransitionHost() :
	_state(nullptr),
	_resources(nullptr) {
}

void GameplayRoomTransitionHost::bind(GameplayMainPlaceState *state,
                                      GameplayResourceHost *resources) {
	_state = state;
	_resources = resources;
}

void GameplayRoomTransitionHost::clear() {
	_state = nullptr;
	_resources = nullptr;
}

bool GameplayRoomTransitionHost::validateTransitionDescriptor(
		const GameplayRoomDescriptor &room,
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) const {
	if (!room.name.equalsIgnoreCase(transition.targetRoom)) {
		errorMessage = Common::String::format(
			"Resolved transition targets '%s' but Room registry returned '%s'",
			transition.targetRoom.c_str(), room.name.c_str());
		return false;
	}

	if (!sameWhenBothPresent(room.prefix, transition.targetPrefix) ||
	    !sameWhenBothPresent(room.background, transition.targetBackground) ||
	    !sameWhenBothPresent(room.navigationMap, transition.targetMap) ||
	    !sameWhenBothPresent(room.cameraMap, transition.targetCameraMap) ||
	    !sameWhenBothPresent(room.camera, transition.targetCamera)) {
		errorMessage = Common::String::format(
			"Resolved transition resources disagree with Room '%s'",
			room.name.c_str());
		return false;
	}

	return true;
}

bool GameplayRoomTransitionHost::activateResolvedRoom(
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) {
	errorMessage.clear();

	if (!_state || !_resources) {
		errorMessage = "Gameplay room transition host is not bound";
		return false;
	}
	if (_state->mainPlace.empty()) {
		errorMessage = "Gameplay MainPlace is not active";
		return false;
	}

	const int32 roomId = _state->rooms.find(transition.targetRoom);
	const GameplayRoomDescriptor *registered = _state->rooms.room(roomId);
	if (!registered) {
		errorMessage = Common::String::format(
			"Target Room '%s' is not in the active MainPlace registry",
			transition.targetRoom.c_str());
		return false;
	}

	if (!validateTransitionDescriptor(*registered, transition, errorMessage))
		return false;

	GameplayRoomDescriptor preparedRoom = *registered;

	// Preserve the richer Stage 22 evidence when the older Stage 7 parser did
	// not populate a field. Never overwrite a conflicting non-empty value.
	if (preparedRoom.prefix.empty())
		preparedRoom.prefix = transition.targetPrefix;
	if (preparedRoom.background.empty())
		preparedRoom.background = transition.targetBackground;
	if (preparedRoom.navigationMap.empty())
		preparedRoom.navigationMap = transition.targetMap;
	if (preparedRoom.cameraMap.empty())
		preparedRoom.cameraMap = transition.targetCameraMap;
	if (preparedRoom.camera.empty())
		preparedRoom.camera = transition.targetCamera;

	const Common::Path navPath =
		gameplayNavigationPath(_state->mainPlace, preparedRoom);
	if (navPath.empty()) {
		errorMessage = Common::String::format(
			"Target Room '%s' has no navigation map",
			preparedRoom.name.c_str());
		return false;
	}

	Common::String bspText;
	if (!_resources->readPlainText(navPath, bspText)) {
		errorMessage = Common::String::format(
			"Unable to read target Room navigation %s",
			navPath.toString().c_str());
		return false;
	}

	BspNavigation preparedNavigation;
	if (!preparedNavigation.parse(bspText, errorMessage))
		return false;

	// Commit only after every check and parse has succeeded.
	_state->activeRoom = preparedRoom;
	_state->navigation = preparedNavigation;
	return true;
}

} // End of namespace ZeroComico
