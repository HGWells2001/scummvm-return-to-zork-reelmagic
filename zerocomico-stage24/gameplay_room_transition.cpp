/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage24/gameplay_room_transition.h"

namespace ZeroComico {

GameplayRoomStateActivation::GameplayRoomStateActivation() :
	_state(nullptr),
	_sceneHost(nullptr) {
}

void GameplayRoomStateActivation::bind(
		GameplayMainPlaceState *state,
		GameplayRoomSceneHost *sceneHost) {
	_state = state;
	_sceneHost = sceneHost;
}

void GameplayRoomStateActivation::clear() {
	_state = nullptr;
	_sceneHost = nullptr;
}

bool GameplayRoomStateActivation::activatePreparedRoom(
		const ResolvedRoomTransition &transition,
		const PreparedRoomNavigation &navigation,
		Common::String &errorMessage) {
	errorMessage.clear();

	if (!_state || !_sceneHost) {
		errorMessage = "GameplayRoomStateActivation is not fully bound";
		return false;
	}

	if (!navigation.roomName.equalsIgnoreCase(transition.targetRoom)) {
		errorMessage = Common::String::format(
			"Prepared navigation Room '%s' does not match target '%s'",
			navigation.roomName.c_str(),
			transition.targetRoom.c_str());
		return false;
	}

	const int32 roomId = _state->rooms.find(transition.targetRoom);
	const GameplayRoomDescriptor *declaredRoom = _state->rooms.room(roomId);
	if (!declaredRoom) {
		errorMessage = Common::String::format(
			"Target Room '%s' is not present in GameplayRoomRegistry",
			transition.targetRoom.c_str());
		return false;
	}

	GameplayRoomDescriptor preparedRoom = *declaredRoom;
	BspNavigation preparedActorNavigation = navigation.actorNavigation;

	if (!_sceneHost->activateGameplayRoomScene(
			transition, navigation, errorMessage))
		return false;

	// Commit only after the entire renderer-side transaction succeeds.
	_state->activeRoom = preparedRoom;
	_state->navigation = preparedActorNavigation;
	return true;
}

GameplayRoomTransitionController::GameplayRoomTransitionController() :
	_prepared(false) {
}

void GameplayRoomTransitionController::clear() {
	_runtime.clear();
	_navigationHost.clear();
	_stateActivation.clear();
	_graph.clear();
	_prepared = false;
}

bool GameplayRoomTransitionController::prepare(
		GameplayMainPlaceState *state,
		const RoomTopologyDocument &topology,
		const ShapeScriptDocument &shapes,
		const ShapeGeometryDocument *geometry,
		GameplayResourceHost *resources,
		GameplayRoomSceneHost *sceneHost,
		Common::String &errorMessage) {
	clear();
	errorMessage.clear();

	if (!state || !resources || !sceneHost) {
		errorMessage = "Gameplay room transition requires state/resources/scene host";
		return false;
	}
	if (state->mainPlace.empty() || state->activeRoom.name.empty()) {
		errorMessage = "GameplayMainPlaceState has no active MainPlace/Room";
		return false;
	}

	RoomTransitionGraphBuilder builder;
	builder.build(topology, shapes, geometry, _graph);

	_stateActivation.bind(state, sceneHost);
	_navigationHost.bind(state->mainPlace, resources, &_stateActivation);
	_runtime.bind(&_graph, &_navigationHost);
	_runtime.setCurrentRoom(state->activeRoom.name);
	_prepared = true;
	return true;
}

RoomTransitionResult GameplayRoomTransitionController::activatePortal(
		const Common::String &portalShape) {
	if (!_prepared)
		return kRoomTransitionNotFound;
	return _runtime.activatePortal(portalShape);
}

} // End of namespace ZeroComico
