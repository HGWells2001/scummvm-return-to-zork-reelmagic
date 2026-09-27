/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage11/actor_motion_controller.h"

#include "zerocomico-stage9/actor_animation_runtime.h"

namespace ZeroComico {

ActorMotionController::ActorMotionController() :
	_animation(nullptr),
	_configured(false),
	_walking(false) {
}

void ActorMotionController::bind(ActorAnimationRuntime *animation) {
	clear();
	_animation = animation;
}

void ActorMotionController::clear() {
	if (_animation)
		_animation->stop();
	_animation = nullptr;
	_clips = ActorMotionClips();
	_configured = false;
	_walking = false;
}

bool ActorMotionController::configure(const ActorMotionClips &clips,
                                      Common::String &errorMessage) {
	errorMessage.clear();
	if (!_animation) {
		errorMessage = "ActorMotionController has no animation runtime";
		return false;
	}
	if (clips.walk.empty()) {
		errorMessage = "Walking clip is unresolved";
		return false;
	}
	if (!_animation->findClip(clips.walk)) {
		errorMessage = Common::String::format(
			"Walking clip '%s' is not present in the actor ANJ",
			clips.walk.c_str());
		return false;
	}
	if (!clips.idle.empty() && !_animation->findClip(clips.idle)) {
		errorMessage = Common::String::format(
			"Idle clip '%s' is not present in the actor ANJ",
			clips.idle.c_str());
		return false;
	}

	_clips = clips;
	_configured = true;
	_walking = false;
	return startCurrentState();
}

bool ActorMotionController::startCurrentState() {
	if (!_configured || !_animation)
		return false;

	if (_walking)
		return _animation->play(_clips.walk, true);

	if (_clips.idle.empty()) {
		_animation->stop();
		return true;
	}
	return _animation->play(_clips.idle, true);
}

bool ActorMotionController::setWalking(bool walking) {
	if (!_configured || !_animation)
		return false;

	const Common::String desired = walking ? _clips.walk : _clips.idle;
	if (_walking == walking) {
		if (desired.empty())
			return true;
		if (_animation->isPlaying() &&
		    _animation->activeClipName().equalsIgnoreCase(desired))
			return true;
	}

	_walking = walking;
	return startCurrentState();
}

void ActorMotionController::update(uint32 deltaMillis) {
	if (_animation)
		_animation->update(deltaMillis);
}

} // End of namespace ZeroComico
