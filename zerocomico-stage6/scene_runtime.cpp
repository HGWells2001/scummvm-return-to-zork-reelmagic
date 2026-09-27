/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

void SceneRegistry::clear() {
	_objects.clear();
	_names.clear();
}

int32 SceneRegistry::addObject(const Common::String &name, SceneObjectType type,
                               const TransformSample &base) {
	// Runtime registry intentionally contains only transformable scene objects.
	// Materials are kept in the renderer/material table, avoiding the F000/F003
	// same-name ambiguity found during Stage 5.
	const int32 existing = findObject(name);
	if (existing >= 0)
		return existing;

	SceneObjectState state;
	state.name = name;
	state.type = type;
	state.baseTransform = base;
	state.currentTransform = base;
	state.scriptVisible = true;

	_objects.push_back(state);
	_names.push_back(name);
	return (int32)_objects.size() - 1;
}

int32 SceneRegistry::findObject(const Common::String &name) const {
	for (uint32 i = 0; i < _objects.size(); ++i) {
		if (_objects[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

SceneObjectState *SceneRegistry::object(int32 id) {
	if (id < 0 || (uint32)id >= _objects.size())
		return nullptr;
	return &_objects[id];
}

const SceneObjectState *SceneRegistry::object(int32 id) const {
	if (id < 0 || (uint32)id >= _objects.size())
		return nullptr;
	return &_objects[id];
}

void SceneRegistry::setVisible(const Common::String &name, bool visible) {
	const int32 id = findObject(name);
	SceneObjectState *state = object(id);
	if (state)
		state->scriptVisible = visible;
}

SceneRuntime::SceneRuntime() :
	_clips(nullptr) {
}

void SceneRuntime::setAnimationClips(const Common::Array<AnimationClip> *clips) {
	_clips = clips;
	_player.stop();
}

const AnimationClip *SceneRuntime::findClip(const Common::String &name) const {
	if (!_clips)
		return nullptr;

	for (uint32 i = 0; i < _clips->size(); ++i) {
		if ((*_clips)[i].name.equalsIgnoreCase(name))
			return &(*_clips)[i];
	}
	return nullptr;
}

void SceneRuntime::update(uint32 deltaMillis) {
	if (_player.clip()) {
		_player.update(deltaMillis);
		applyAnimation();
	}
}

void SceneRuntime::applyAnimation() {
	const AnimationClip *clip = _player.clip();

	for (uint32 i = 0; i < _registry.size(); ++i) {
		SceneObjectState *state = _registry.object((int32)i);
		if (!state)
			continue;

		if (!clip) {
			state->currentTransform = state->baseTransform;
			continue;
		}

		TransformSample sampled;
		if (_player.sample(state->name, state->baseTransform, sampled))
			state->currentTransform = sampled;
		else
			state->currentTransform = state->baseTransform;
	}
}

void SceneRuntime::setObjectVisible(const Common::String &objectName, bool visible) {
	_registry.setVisible(objectName, visible);
}

bool SceneRuntime::playAnimation(const Common::String &animationName, bool loop) {
	const AnimationClip *clip = findClip(animationName);
	if (!clip)
		return false;

	_player.play(clip, loop);
	applyAnimation();
	return true;
}

bool SceneRuntime::isAnimationPlaying() const {
	return _player.isPlaying();
}

void SceneRuntime::setFocus(const Common::String &cameraName) {
	_focusCamera = cameraName;
}

void SceneRuntime::requestMainPlace(const Common::String &mainPlaceName) {
	_pendingMainPlace = normalizeMainPlaceName(mainPlaceName);
}

Common::String SceneRuntime::consumePendingMainPlace() {
	Common::String result = _pendingMainPlace;
	_pendingMainPlace.clear();
	return result;
}

} // End of namespace ZeroComico
