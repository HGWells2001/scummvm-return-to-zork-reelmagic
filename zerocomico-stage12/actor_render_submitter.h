/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE12_ACTOR_RENDER_SUBMITTER_H
#define ZEROCOMICO_STAGE12_ACTOR_RENDER_SUBMITTER_H

#include "zerocomico-stage9/actor_textures.h"
#include "zerocomico-stage12/actor_render_frame.h"

namespace ZeroComico {

class ActorTriangleSink {
public:
	virtual ~ActorTriangleSink() {}

	virtual bool beginActor(const Common::String &actorName) = 0;
	virtual bool submitTriangle(const ActorRenderTriangle &triangle,
	                            const ActorTexture *texture) = 0;
	virtual void endActor() = 0;
};

class ActorRenderSubmitter {
public:
	bool submit(const ActorModel &actor,
	            const ActorTextureSet &textures,
	            const ActorRenderFrame &frame,
	            ActorTriangleSink &sink,
	            Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
