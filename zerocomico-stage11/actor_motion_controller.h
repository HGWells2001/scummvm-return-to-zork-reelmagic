/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE11_ACTOR_MOTION_CONTROLLER_H
#define ZEROCOMICO_STAGE11_ACTOR_MOTION_CONTROLLER_H

#include "common/str.h"

namespace ZeroComico {

class ActorAnimationRuntime;

struct ActorMotionClips {
	Common::String idle;
	Common::String walk;
};

/**
 * Maps semantic gameplay motion (idle/walking) to already decoded ANJ clips.
 *
 * The mapping is injected. Stage 11 deliberately does not assume that a
 * high-level JACS sequence called "walk" is identical to an ANJ clip name;
 * the retail .seq SequenceTable will populate this mapping once decoded.
 */
class ActorMotionController {
public:
	ActorMotionController();

	void bind(ActorAnimationRuntime *animation);
	void clear();

	bool configure(const ActorMotionClips &clips, Common::String &errorMessage);
	bool setWalking(bool walking);
	void update(uint32 deltaMillis);

	bool configured() const { return _configured; }
	bool walking() const { return _walking; }
	const ActorMotionClips &clips() const { return _clips; }

private:
	bool startCurrentState();

	ActorAnimationRuntime *_animation;
	ActorMotionClips _clips;
	bool _configured;
	bool _walking;
};

} // End of namespace ZeroComico

#endif
