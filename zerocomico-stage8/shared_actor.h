/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE8_SHARED_ACTOR_H
#define ZEROCOMICO_STAGE8_SHARED_ACTOR_H

#include "common/path.h"
#include "common/str.h"

namespace ZeroComico {

struct SharedActorAssets {
	Common::String name;
	Common::Path model;
	Common::Path animation;
	Common::Path sequence;
	Common::Path material;

	bool valid() const {
		return !name.empty() && !model.empty() && !animation.empty();
	}
};

/**
 * Zero Comico keeps the three main playable actors in Mpx/bodies.
 * Build their resource paths from the actor name instead of duplicating
 * MainPlace-local copies.
 */
SharedActorAssets makeSharedActorAssets(const Common::String &actorName);

} // End of namespace ZeroComico

#endif
