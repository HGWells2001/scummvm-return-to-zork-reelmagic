/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/animation_player.h"

namespace ZeroComico {

AnimationPlayer::AnimationPlayer() :
	_clip(nullptr),
	_frame(0.0f),
	_loop(false),
	_playing(false),
	_finished(false) {
}

void AnimationPlayer::play(const AnimationClip *clip, bool loop) {
	_clip = clip;
	_loop = loop;
	_finished = false;
	_playing = (_clip != nullptr);
	_frame = _clip ? (float)_clip->firstFrame : 0.0f;
}

void AnimationPlayer::stop() {
	_playing = false;
	_finished = (_clip != nullptr);
}

void AnimationPlayer::update(uint32 deltaMillis) {
	if (!_playing || !_clip)
		return;

	const float fps = _clip->framesPerSecond > 0.0f ? _clip->framesPerSecond : 25.0f;
	_frame += ((float)deltaMillis * fps) / 1000.0f;

	if (_frame <= _clip->lastFrame)
		return;

	if (_loop && _clip->lastFrame >= _clip->firstFrame) {
		const float span = (float)(_clip->lastFrame - _clip->firstFrame + 1);
		if (span > 0.0f) {
			while (_frame > _clip->lastFrame)
				_frame -= span;
		}
	} else {
		_frame = (float)_clip->lastFrame;
		_playing = false;
		_finished = true;
	}
}

const ObjectTimeline *AnimationPlayer::findTrack(const Common::String &objectName) const {
	if (!_clip)
		return nullptr;

	for (uint32 i = 0; i < _clip->tracks.size(); ++i) {
		if (_clip->tracks[i].objectName.equalsIgnoreCase(objectName))
			return &_clip->tracks[i];
	}
	return nullptr;
}

bool AnimationPlayer::sample(const Common::String &objectName,
                             const TransformSample &base,
                             TransformSample &out) const {
	const ObjectTimeline *track = findTrack(objectName);
	if (!track)
		return false;

	out = base;
	out.translation = evaluateTcbVec3(track->translationKeys, _frame, base.translation);
	out.scale = evaluateTcbVec3(track->scaleKeys, _frame, base.scale);
	out.rotation = evaluateAxisAngle(track->rotationKeys, _frame, base.rotation);
	out.visible = evaluateVisibility(track->visibilityKeys, _frame, base.visible);
	return true;
}

} // End of namespace ZeroComico
