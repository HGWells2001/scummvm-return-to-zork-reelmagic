/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ACTOR_TEXTURES_H
#define ZEROCOMICO_STAGE9_ACTOR_TEXTURES_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage9/resource_decoder.h"

namespace ZeroComico {

struct ActorTexture {
	int32 materialIndex;
	Common::String resourceName;
	Common::Path resolvedPath;
	JGF5Image image;

	ActorTexture() : materialIndex(-1) {}
};

class ActorTextureSet {
public:
	bool load(const ActorModel &actor,
	          ActorPackedResourceHost &host,
	          Common::String &errorMessage);

	const ActorTexture *forMaterial(int32 materialIndex) const;
	const Common::Array<Common::Path> &missing() const { return _missing; }
	const Common::Array<ActorTexture> &textures() const { return _textures; }

	void clear();

private:
	Common::Array<ActorTexture> _textures;
	Common::Array<Common::Path> _missing;
};

Common::Path actorRelativeResourcePath(const SharedActorAssets &assets,
                                       const Common::String &resourceName);

} // End of namespace ZeroComico

#endif
