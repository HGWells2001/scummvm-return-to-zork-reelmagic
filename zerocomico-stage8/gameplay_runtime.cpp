/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cmath>

#include "zerocomico-stage8/gameplay_runtime.h"

namespace ZeroComico {

namespace {

static float distanceSquared(const NavVec2 &a, const NavVec2 &b) {
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	return dx * dx + dy * dy;
}

} // namespace

GameplayRuntime::GameplayRuntime() :
	_state(nullptr),
	_host(nullptr),
	_bridge(nullptr),
	_hasActorPosition(false),
	_walkingForInteraction(false),
	_walkSpeed(0.0f) {
}

void GameplayRuntime::clear() {
	if (_host && !_actorAssets.name.empty())
		_host->setActorWalking(_actorAssets.name, false);

	_state = nullptr;
	_host = nullptr;
	_bridge = nullptr;
	_actorAssets = SharedActorAssets();
	_variables.clear();
	_handlers.clear();
	_handlerVm.clear();
	_interaction.setRegistry(nullptr);
	_path.clear();
	_actorPosition = NavVec2();
	_hasActorPosition = false;
	_walkingForInteraction = false;
	_walkSpeed = 0.0f;
}

bool GameplayRuntime::prepare(GameplayMainPlaceState *state,
                              const Common::String &decodedRoomScript,
                              const Common::String &decodedPuzzleScript,
                              GameplayRuntimeHost *host,
                              ScriptBridge *bridge,
                              Common::String &errorMessage) {
	clear();
	errorMessage.clear();

	if (!state || !host || !bridge) {
		errorMessage = "GameplayRuntime requires state, host and ScriptBridge";
		return false;
	}

	SharedActorAssets actor = makeSharedActorAssets("Giovanni");
	if (!actor.valid() || !host->loadSharedActor(actor)) {
		errorMessage = "Unable to load shared actor Giovanni";
		return false;
	}

	_state = state;
	_host = host;
	_bridge = bridge;
	_actorAssets = actor;

	_variables.parseDeclarations(decodedRoomScript);
	_variables.parseDeclarations(decodedPuzzleScript);
	_handlers.parse(decodedPuzzleScript);
	_interaction.setRegistry(&_state->objects);

	return true;
}

void GameplayRuntime::setActorStartPosition(const NavVec2 &position) {
	_actorPosition = position;
	_hasActorPosition = true;
	if (_host)
		_host->setActorFloorPosition(_actorAssets.name, _actorPosition);
}

bool GameplayRuntime::beginPath(const NavVec2 &goal) {
	if (!_state || !_host || !_hasActorPosition)
		return false;

	if (_walkSpeed <= 0.0f)
		return false;
	if (!_path.begin(_state->navigation, _actorPosition, goal, _walkSpeed))
		return false;

	_host->setActorWalking(_actorAssets.name, true);
	return true;
}

bool GameplayRuntime::walkToFloorPoint(const NavVec2 &point) {
	_walkingForInteraction = false;
	return beginPath(point);
}

bool GameplayRuntime::resolveObjectGoal(const Common::String &entityName,
                                        float range,
                                        NavVec2 &goal) const {
	if (!_host || !_hasActorPosition)
		return false;

	NavVec2 target;
	if (!_host->resolveEntityFloorPosition(entityName, target))
		return false;

	if (range <= 0.0f) {
		goal = target;
		return true;
	}

	const float dx = _actorPosition.x - target.x;
	const float dy = _actorPosition.y - target.y;
	const float length = std::sqrt(dx * dx + dy * dy);
	if (length <= range) {
		goal = _actorPosition;
		return true;
	}

	if (length <= 0.0001f) {
		goal = target;
		return true;
	}

	goal.x = target.x + dx * (range / length);
	goal.y = target.y + dy * (range / length);
	return true;
}

bool GameplayRuntime::isWithinObjectRange(const Common::String &entityName,
                                          float range) const {
	if (!_host || !_hasActorPosition)
		return false;

	NavVec2 target;
	if (!_host->resolveEntityFloorPosition(entityName, target))
		return false;

	return distanceSquared(_actorPosition, target) <= range * range;
}

bool GameplayRuntime::beginWalkToObject(const Common::String &entityName,
                                        float range) {
	NavVec2 goal;
	if (!resolveObjectGoal(entityName, range, goal))
		return false;

	_walkingForInteraction = true;
	if (distanceSquared(goal, _actorPosition) <= 0.0001f)
		return true;

	if (!beginPath(goal)) {
		_walkingForInteraction = false;
		return false;
	}
	return true;
}

GameplayInteractionResult GameplayRuntime::interact(
		const Common::String &pickedSceneEntity, GameplayVerb verb) {
	if (!_state)
		return kGameplayInteractionNoObject;
	return _interaction.request(pickedSceneEntity, verb, *this);
}

bool GameplayRuntime::executeObjectHandler(const Common::String &objectName,
                                           GameplayVerb verb) {
	if (!_state || !_host)
		return false;

	const ObjectHandlerBody *body = _handlers.find(objectName, verb);
	if (!body) {
		if (verb == kGameplayExamine) {
			const int32 id = _state->objects.findByName(objectName);
			const GameplayObject *object = _state->objects.object(id);
			if (object && !object->examineText.empty()) {
				_host->showExamineText(object->examineText);
				return true;
			}
		}
		return false;
	}

	_handlerVm.begin(body, &_variables, _bridge, this);
	return true;
}

bool GameplayRuntime::startDialog(const Common::String &speaker,
                                  const Common::String &dialogName) {
	return _host && _host->startDialog(speaker, dialogName);
}

bool GameplayRuntime::isDialogPlaying() const {
	return _host && _host->isDialogPlaying();
}

void GameplayRuntime::update(uint32 deltaMillis) {
	if (_path.active()) {
		_path.update(deltaMillis);
		_actorPosition = _path.position();
		if (_host)
			_host->setActorFloorPosition(_actorAssets.name, _actorPosition);

		if (!_path.active()) {
			if (_host)
				_host->setActorWalking(_actorAssets.name, false);

			if (_walkingForInteraction) {
				_walkingForInteraction = false;
				_interaction.finishWalk(true, *this);
			}
		}
	}

	if (_handlerVm.active())
		_handlerVm.update();
}

} // End of namespace ZeroComico
