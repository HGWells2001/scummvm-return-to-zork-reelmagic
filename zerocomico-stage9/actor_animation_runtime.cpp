/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage9/actor_animation_runtime.h"

namespace ZeroComico {

ActorAnimationRuntime::ActorAnimationRuntime() :
	_actor(nullptr),
	_registry(nullptr) {
}

void ActorAnimationRuntime::bind(const ActorModel *actor, SceneRegistry *registry) {
	clear();
	_actor = actor;
	_registry = registry;
}

void ActorAnimationRuntime::clear() {
	_player.stop();
	_actor = nullptr;
	_registry = nullptr;
}

const AnimationClip *ActorAnimationRuntime::findClip(const Common::String &name) const {
	if (!_actor)
		return nullptr;

	for (uint32 i = 0; i < _actor->animationClips.size(); ++i) {
		if (_actor->animationClips[i].name.equalsIgnoreCase(name))
			return &_actor->animationClips[i];
	}
	return nullptr;
}

bool ActorAnimationRuntime::play(const Common::String &clipName, bool loop) {
	const AnimationClip *clip = findClip(clipName);
	if (!clip)
		return false;

	_player.play(clip, loop);
	apply();
	return true;
}

void ActorAnimationRuntime::stop() {
	_player.stop();
}

Common::String ActorAnimationRuntime::activeClipName() const {
	return _player.clip() ? _player.clip()->name : Common::String();
}

void ActorAnimationRuntime::update(uint32 deltaMillis) {
	if (!_player.clip())
		return;

	_player.update(deltaMillis);
	apply();
}

void ActorAnimationRuntime::apply() {
	if (!_actor || !_registry || !_player.clip())
		return;

	for (uint32 i = 0; i < _actor->animationClips.size(); ++i) {
		// Only the clip owned by the player can contribute this tick.
		if (&_actor->animationClips[i] != _player.clip())
			continue;

		const AnimationClip &clip = _actor->animationClips[i];
		for (uint32 t = 0; t < clip.tracks.size(); ++t) {
			const ObjectTimeline &track = clip.tracks[t];
			const int32 objectId = _registry->findObject(track.objectName);
			SceneObjectState *object = _registry->object(objectId);
			if (!object)
				continue;

			TransformSample sampled;
			if (_player.sample(track.objectName, object->baseTransform, sampled))
				object->currentTransform = sampled;
		}
		break;
	}
}

} // End of namespace ZeroComico
