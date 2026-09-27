/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ACTOR_ANIMATION_RUNTIME_H
#define ZEROCOMICO_STAGE9_ACTOR_ANIMATION_RUNTIME_H

#include "common/str.h"

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage9/actor_model.h"

namespace ZeroComico {

class ActorAnimationRuntime {
public:
	ActorAnimationRuntime();

	void bind(const ActorModel *actor, SceneRegistry *registry);
	void clear();

	bool play(const Common::String &clipName, bool loop);
	void stop();
	void update(uint32 deltaMillis);

	bool isPlaying() const { return _player.isPlaying(); }
	Common::String activeClipName() const;
	const AnimationClip *findClip(const Common::String &name) const;

private:
	void apply();

	const ActorModel *_actor;
	SceneRegistry *_registry;
	AnimationPlayer _player;
};

} // End of namespace ZeroComico

#endif
