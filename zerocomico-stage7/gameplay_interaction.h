/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE7_GAMEPLAY_INTERACTION_H
#define ZEROCOMICO_STAGE7_GAMEPLAY_INTERACTION_H

#include "common/str.h"

#include "zerocomico-stage7/gameplay_object.h"

namespace ZeroComico {

enum GameplayVerb {
	kGameplayExamine,
	kGameplayOperate,
	kGameplayTake
};

enum GameplayInteractionResult {
	kGameplayInteractionNoObject,
	kGameplayInteractionDisabled,
	kGameplayInteractionUnsupported,
	kGameplayInteractionWalking,
	kGameplayInteractionExecuted,
	kGameplayInteractionWalkFailed
};

class GameplayInteractionHost {
public:
	virtual ~GameplayInteractionHost() {}

	virtual bool isWithinObjectRange(const Common::String &entityName,
	                                 float range) const = 0;
	virtual bool beginWalkToObject(const Common::String &entityName,
	                               float range) = 0;
	virtual bool executeObjectHandler(const Common::String &objectName,
	                                  GameplayVerb verb) = 0;
};

class GameplayInteractionController {
public:
	GameplayInteractionController();

	void setRegistry(const GameplayObjectRegistry *registry) { _registry = registry; }

	GameplayInteractionResult request(const Common::String &pickedSceneEntity,
	                                  GameplayVerb verb,
	                                  GameplayInteractionHost &host);

	GameplayInteractionResult finishWalk(bool reached,
	                                     GameplayInteractionHost &host);

	bool hasPendingInteraction() const { return _pendingObject >= 0; }

private:
	bool supported(const GameplayObject &object, GameplayVerb verb) const;
	float reachFor(const GameplayObject &object, GameplayVerb verb) const;
	void clearPending();

	const GameplayObjectRegistry *_registry;
	int32 _pendingObject;
	GameplayVerb _pendingVerb;
};

} // End of namespace ZeroComico

#endif
