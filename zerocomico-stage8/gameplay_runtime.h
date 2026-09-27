/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE8_GAMEPLAY_RUNTIME_H
#define ZEROCOMICO_STAGE8_GAMEPLAY_RUNTIME_H

#include "common/str.h"

#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage7/gameplay_interaction.h"
#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage7/path_follower.h"
#include "zerocomico-stage8/gameplay_variables.h"
#include "zerocomico-stage8/object_handler_vm.h"
#include "zerocomico-stage8/object_handlers.h"
#include "zerocomico-stage8/shared_actor.h"

namespace ZeroComico {

class GameplayRuntimeHost : public ObjectHandlerExternalOpcodeHost {
public:
	virtual ~GameplayRuntimeHost() {}

	ObjectHandlerExternalOpcodeResult executeObjectHandlerOpcode(
		const Common::String &,
		const Common::Array<Common::String> &) override {
		return kObjectHandlerExternalUnhandled;
	}

	virtual bool loadSharedActor(const SharedActorAssets &assets) = 0;

	virtual bool resolveEntityFloorPosition(const Common::String &entityName,
	                                        NavVec2 &position) const = 0;
	virtual void setActorFloorPosition(const Common::String &actorName,
	                                   const NavVec2 &position) = 0;
	virtual void setActorWalking(const Common::String &actorName,
	                             bool walking) = 0;

	virtual bool startDialog(const Common::String &speaker,
	                         const Common::String &dialogName) = 0;
	virtual bool isDialogPlaying() const = 0;
	virtual void showExamineText(const Common::String &text) = 0;
};

class GameplayRuntime :
	public GameplayInteractionHost,
	public GameplayHandlerHost {
public:
	GameplayRuntime();

	bool prepare(GameplayMainPlaceState *state,
	             const Common::String &decodedRoomScript,
	             const Common::String &decodedPuzzleScript,
	             GameplayRuntimeHost *host,
	             ScriptBridge *bridge,
	             Common::String &errorMessage);

	void clear();

	/**
	 * The retail start position must be supplied by the engine after resolving
	 * its helper/vector or preceding cut-scene. Stage 8 deliberately has no
	 * guessed coordinate fallback.
	 */
	void setActorStartPosition(const NavVec2 &position);
	void setWalkSpeed(float unitsPerSecond) { _walkSpeed = unitsPerSecond; }
	float walkSpeed() const { return _walkSpeed; }

	bool walkToFloorPoint(const NavVec2 &point);
	GameplayInteractionResult interact(const Common::String &pickedSceneEntity,
	                                   GameplayVerb verb);

	void update(uint32 deltaMillis);

	const NavVec2 &actorPosition() const { return _actorPosition; }
	bool hasActorPosition() const { return _hasActorPosition; }
	bool walking() const { return _path.active(); }

	GameplayVariables &variables() { return _variables; }
	const GameplayVariables &variables() const { return _variables; }

	const ObjectHandlerVM &handlerVM() const { return _handlerVm; }
	const SharedActorAssets &actorAssets() const { return _actorAssets; }

	// GameplayInteractionHost
	bool isWithinObjectRange(const Common::String &entityName,
	                         float range) const override;
	bool beginWalkToObject(const Common::String &entityName,
	                       float range) override;
	bool executeObjectHandler(const Common::String &objectName,
	                          GameplayVerb verb) override;

	// GameplayHandlerHost
	bool startDialog(const Common::String &speaker,
	                 const Common::String &dialogName) override;
	bool isDialogPlaying() const override;

private:
	bool beginPath(const NavVec2 &goal);
	bool resolveObjectGoal(const Common::String &entityName,
	                       float range,
	                       NavVec2 &goal) const;

	GameplayMainPlaceState *_state;
	GameplayRuntimeHost *_host;
	ScriptBridge *_bridge;
	SharedActorAssets _actorAssets;
	GameplayVariables _variables;
	ObjectHandlerRegistry _handlers;
	ObjectHandlerVM _handlerVm;
	GameplayInteractionController _interaction;
	PathFollower _path;
	NavVec2 _actorPosition;
	bool _hasActorPosition;
	bool _walkingForInteraction;
	float _walkSpeed;
};

} // End of namespace ZeroComico

#endif
