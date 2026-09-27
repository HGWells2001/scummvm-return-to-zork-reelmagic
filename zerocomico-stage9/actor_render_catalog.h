/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ACTOR_RENDER_CATALOG_H
#define ZEROCOMICO_STAGE9_ACTOR_RENDER_CATALOG_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage9/actor_model.h"

namespace ZeroComico {

struct ActorRenderBatch {
	uint32 meshIndex;
	uint32 firstTriangle;
	uint32 triangleCount;
	int32 materialIndex;
	Common::String materialName;
	Common::String textureResource;

	ActorRenderBatch() :
		meshIndex(0), firstTriangle(0), triangleCount(0), materialIndex(-1) {}
};

class ActorRenderCatalog {
public:
	bool build(const ActorModel &actor, Common::String &errorMessage);

	const Common::Array<uint32> &renderableMeshes() const { return _renderableMeshes; }
	const Common::Array<ActorRenderBatch> &batches() const { return _batches; }

private:
	Common::Array<uint32> _renderableMeshes;
	Common::Array<ActorRenderBatch> _batches;
};

} // End of namespace ZeroComico

#endif
