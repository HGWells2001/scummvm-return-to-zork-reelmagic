/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE24_GAMEPLAY_ROOM_TRANSITION_H
#define ZEROCOMICO_STAGE24_GAMEPLAY_ROOM_TRANSITION_H

#include "common/str.h"

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage19/shape_geometry.h"
#include "zerocomico-stage21/room_topology.h"
#include "zerocomico-stage22/room_transition.h"
#include "zerocomico-stage23/room_navigation_bundle.h"

namespace ZeroComico {

class GameplayRoomSceneHost {
public:
	virtual ~GameplayRoomSceneHost() {}

	/**
	 * Prepare/activate the renderer-side target Room. Returning false leaves
	 * GameplayMainPlaceState untouched.
	 */
	virtual bool activateGameplayRoomScene(
		const ResolvedRoomTransition &transition,
		const PreparedRoomNavigation &navigation,
		Common::String &errorMessage) = 0;
};

/**
 * Stage 23 activation adapter that commits the Stage 7 gameplay state only
 * after renderer-side activation succeeds.
 */
class GameplayRoomStateActivation : public RoomNavigationActivationHost {
public:
	GameplayRoomStateActivation();

	void bind(GameplayMainPlaceState *state,
	          GameplayRoomSceneHost *sceneHost);

	void clear();

	bool activatePreparedRoom(
		const ResolvedRoomTransition &transition,
		const PreparedRoomNavigation &navigation,
		Common::String &errorMessage) override;

private:
	GameplayMainPlaceState *_state;
	GameplayRoomSceneHost *_sceneHost;
};

/**
 * Full data-driven room-transition stack:
 *
 * Room portal evidence -> unambiguous graph -> Map/MapCam preparation ->
 * renderer activation -> GameplayMainPlaceState commit.
 */
class GameplayRoomTransitionController {
public:
	GameplayRoomTransitionController();

	bool prepare(GameplayMainPlaceState *state,
	             const RoomTopologyDocument &topology,
	             const ShapeScriptDocument &shapes,
	             const ShapeGeometryDocument *geometry,
	             GameplayResourceHost *resources,
	             GameplayRoomSceneHost *sceneHost,
	             Common::String &errorMessage);

	void clear();

	RoomTransitionResult activatePortal(const Common::String &portalShape);

	const Common::String &currentRoom() const {
		return _runtime.currentRoom();
	}

	const Common::String &lastError() const {
		return _runtime.lastError();
	}

	const PreparedRoomNavigation &currentNavigation() const {
		return _navigationHost.currentNavigation();
	}

	const RoomTransitionGraph &graph() const { return _graph; }

private:
	RoomTransitionGraph _graph;
	GameplayRoomStateActivation _stateActivation;
	RoomNavigationTransitionHost _navigationHost;
	RoomTransitionRuntime _runtime;
	bool _prepared;
};

} // End of namespace ZeroComico

#endif
