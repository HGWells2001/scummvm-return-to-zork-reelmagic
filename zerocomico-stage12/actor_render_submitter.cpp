/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage12/actor_render_submitter.h"

namespace ZeroComico {

bool ActorRenderSubmitter::submit(const ActorModel &actor,
                                  const ActorTextureSet &textures,
                                  const ActorRenderFrame &frame,
                                  ActorTriangleSink &sink,
                                  Common::String &errorMessage) const {
	errorMessage.clear();
	if (!sink.beginActor(actor.assets.name)) {
		errorMessage = Common::String::format(
			"Renderer rejected actor '%s'", actor.assets.name.c_str());
		return false;
	}

	for (uint32 i = 0; i < frame.triangles.size(); ++i) {
		const ActorRenderTriangle &triangle = frame.triangles[i];
		const ActorTexture *texture = triangle.materialIndex >= 0
			? textures.forMaterial(triangle.materialIndex)
			: nullptr;

		if (!sink.submitTriangle(triangle, texture)) {
			sink.endActor();
			errorMessage = Common::String::format(
				"Renderer rejected actor triangle %u", (uint)i);
			return false;
		}
	}

	sink.endActor();
	return true;
}

} // End of namespace ZeroComico
