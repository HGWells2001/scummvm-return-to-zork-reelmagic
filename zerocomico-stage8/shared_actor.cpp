/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage8/shared_actor.h"

namespace ZeroComico {

SharedActorAssets makeSharedActorAssets(const Common::String &actorName) {
	SharedActorAssets out;
	if (actorName.empty())
		return out;

	out.name = actorName;
	const Common::String base = Common::String::format("Mpx/bodies/%s/%s",
		actorName.c_str(), actorName.c_str());

	out.model = Common::Path(base + ".p3d");
	out.animation = Common::Path(base + ".anj");
	out.sequence = Common::Path(base + ".seq");
	out.material = Common::Path(base + ".mat");
	return out;
}

} // End of namespace ZeroComico
