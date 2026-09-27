/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage7/gameplay_interaction.h"

namespace ZeroComico {

GameplayInteractionController::GameplayInteractionController() :
	_registry(nullptr),
	_pendingObject(-1),
	_pendingVerb(kGameplayExamine) {
}

bool GameplayInteractionController::supported(const GameplayObject &object,
                                              GameplayVerb verb) const {
	if (!object.enabled)
		return false;

	switch (verb) {
	case kGameplayExamine:
		return object.examinable || object.hasExamineHandler || !object.examineText.empty();
	case kGameplayOperate:
		return object.operated || object.hasOperateHandler;
	case kGameplayTake:
		return object.pickable;
	default:
		return false;
	}
}

float GameplayInteractionController::reachFor(const GameplayObject &object,
                                               GameplayVerb verb) const {
	if (verb == kGameplayOperate || verb == kGameplayTake) {
		if (object.operateRange > 0.0f)
			return object.operateRange;
	}
	return object.range;
}

void GameplayInteractionController::clearPending() {
	_pendingObject = -1;
	_pendingVerb = kGameplayExamine;
}

GameplayInteractionResult GameplayInteractionController::request(
		const Common::String &pickedSceneEntity, GameplayVerb verb,
		GameplayInteractionHost &host) {
	clearPending();
	if (!_registry)
		return kGameplayInteractionNoObject;

	const int32 id = _registry->resolvePickedEntity(pickedSceneEntity);
	const GameplayObject *object = _registry->object(id);
	if (!object)
		return kGameplayInteractionNoObject;
	if (!object->enabled)
		return kGameplayInteractionDisabled;
	if (!supported(*object, verb))
		return kGameplayInteractionUnsupported;

	const Common::String target = object->entity.empty() ? object->name : object->entity;
	const float reach = reachFor(*object, verb);

	if (reach > 0.0f && !host.isWithinObjectRange(target, reach)) {
		if (!host.beginWalkToObject(target, reach))
			return kGameplayInteractionWalkFailed;
		_pendingObject = id;
		_pendingVerb = verb;
		return kGameplayInteractionWalking;
	}

	return host.executeObjectHandler(object->name, verb)
		? kGameplayInteractionExecuted
		: kGameplayInteractionUnsupported;
}

GameplayInteractionResult GameplayInteractionController::finishWalk(
		bool reached, GameplayInteractionHost &host) {
	if (!_registry || _pendingObject < 0)
		return kGameplayInteractionNoObject;

	const GameplayObject *object = _registry->object(_pendingObject);
	const GameplayVerb verb = _pendingVerb;
	clearPending();

	if (!reached || !object)
		return kGameplayInteractionWalkFailed;

	return host.executeObjectHandler(object->name, verb)
		? kGameplayInteractionExecuted
		: kGameplayInteractionUnsupported;
}

} // End of namespace ZeroComico
