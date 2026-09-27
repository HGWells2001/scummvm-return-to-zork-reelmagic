/* Zero Comico Stage 11 motion bridge self-test. */

#include <cassert>

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage9/actor_animation_runtime.h"
#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage11/actor_motion_controller.h"

using namespace ZeroComico;

static AnimationClip makeClip(const char *name) {
	AnimationClip clip;
	clip.name = name;
	clip.framesPerSecond = 25.0f;
	clip.firstFrame = 0;
	clip.lastFrame = 24;
	return clip;
}

static void testInjectedMotionNames() {
	ActorModel actor;
	actor.animationClips.push_back(makeClip("gio_idle_resolved"));
	actor.animationClips.push_back(makeClip("gio_walk_resolved"));

	SceneRegistry registry;
	ActorAnimationRuntime animation;
	animation.bind(&actor, &registry);

	ActorMotionController motion;
	motion.bind(&animation);

	ActorMotionClips clips;
	clips.idle = "gio_idle_resolved";
	clips.walk = "gio_walk_resolved";

	Common::String error;
	assert(motion.configure(clips, error));
	assert(error.empty());
	assert(motion.configured());
	assert(!motion.walking());
	assert(animation.isPlaying());
	assert(animation.activeClipName() == "gio_idle_resolved");

	assert(motion.setWalking(true));
	assert(motion.walking());
	assert(animation.activeClipName() == "gio_walk_resolved");

	// Repeating the same state must not restart or lose the selected clip.
	motion.update(200);
	assert(motion.setWalking(true));
	assert(animation.activeClipName() == "gio_walk_resolved");

	assert(motion.setWalking(false));
	assert(!motion.walking());
	assert(animation.activeClipName() == "gio_idle_resolved");
}

static void testNoGuessedWalkFallback() {
	ActorModel actor;
	actor.animationClips.push_back(makeClip("some_real_clip"));

	SceneRegistry registry;
	ActorAnimationRuntime animation;
	animation.bind(&actor, &registry);

	ActorMotionController motion;
	motion.bind(&animation);

	ActorMotionClips unresolved;
	Common::String error;
	assert(!motion.configure(unresolved, error));
	assert(!error.empty());
	assert(!motion.configured());

	ActorMotionClips wrong;
	wrong.walk = "Walk";
	error.clear();
	assert(!motion.configure(wrong, error));
	assert(!error.empty());
	assert(!motion.configured());
}

static void testIdleCanRemainUnresolved() {
	ActorModel actor;
	actor.animationClips.push_back(makeClip("resolved_walk"));

	SceneRegistry registry;
	ActorAnimationRuntime animation;
	animation.bind(&actor, &registry);

	ActorMotionController motion;
	motion.bind(&animation);

	ActorMotionClips clips;
	clips.walk = "resolved_walk";

	Common::String error;
	assert(motion.configure(clips, error));
	assert(!animation.isPlaying());

	assert(motion.setWalking(true));
	assert(animation.activeClipName() == "resolved_walk");

	assert(motion.setWalking(false));
	assert(!animation.isPlaying());
}

int main() {
	testInjectedMotionNames();
	testNoGuessedWalkFallback();
	testIdleCanRemainUnresolved();
	return 0;
}
