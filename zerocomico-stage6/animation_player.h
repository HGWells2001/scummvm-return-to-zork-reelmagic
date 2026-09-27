/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_ANIMATION_PLAYER_H
#define ZEROCOMICO_STAGE6_ANIMATION_PLAYER_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/timeline_eval.h"

namespace ZeroComico {

struct ObjectTimeline {
	Common::String objectName;
	Common::Array<TcbVec3Key> translationKeys;
	Common::Array<TcbVec3Key> scaleKeys;
	Common::Array<TcbAxisAngleKey> rotationKeys;
	Common::Array<VisibilityKey> visibilityKeys;
};

struct AnimationClip {
	Common::String name;
	float framesPerSecond;
	int32 firstFrame;
	int32 lastFrame;
	Common::Array<ObjectTimeline> tracks;

	AnimationClip() : framesPerSecond(25.0f), firstFrame(0), lastFrame(0) {}
};

class AnimationPlayer {
public:
	AnimationPlayer();

	void play(const AnimationClip *clip, bool loop = false);
	void stop();
	void update(uint32 deltaMillis);

	bool isPlaying() const { return _playing; }
	bool isFinished() const { return _finished; }
	float frame() const { return _frame; }
	const AnimationClip *clip() const { return _clip; }

	/**
	 * Sample one named object from the active clip.
	 * Returns false when the clip has no track for that object.
	 */
	bool sample(const Common::String &objectName,
	            const TransformSample &base,
	            TransformSample &out) const;

private:
	const ObjectTimeline *findTrack(const Common::String &objectName) const;

	const AnimationClip *_clip;
	float _frame;
	bool _loop;
	bool _playing;
	bool _finished;
};

} // End of namespace ZeroComico

#endif
