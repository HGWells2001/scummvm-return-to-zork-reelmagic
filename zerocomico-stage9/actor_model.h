/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ACTOR_MODEL_H
#define ZEROCOMICO_STAGE9_ACTOR_MODEL_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage8/shared_actor.h"
#include "zerocomico-stage9/anj_document.h"
#include "zerocomico-stage9/anj_tracks.h"
#include "zerocomico-stage9/p3d_model.h"
#include "zerocomico-stage9/resource_decoder.h"

namespace ZeroComico {

class ActorBinaryResourceHost {
public:
	virtual ~ActorBinaryResourceHost() {}

	virtual bool readDecodedBinary(const Common::Path &path,
	                               Common::Array<byte> &decoded) = 0;
};

class ActorPackedResourceHost {
public:
	virtual ~ActorPackedResourceHost() {}

	/** Read the exact bytes stored on disc, including the JFX1 wrapper. */
	virtual bool readBinary(const Common::Path &path,
	                        Common::Array<byte> &packed) = 0;
};

struct ActorModel {
	SharedActorAssets assets;
	P3DModel model;
	ANJDocument animationDocument;
	Common::Array<AnimationClip> animationClips;
	ANJDecodeStats animationStats;

	void clear();
};

class ActorModelLoader {
public:
	bool load(const SharedActorAssets &assets,
	          ActorBinaryResourceHost &host,
	          ActorModel &out,
	          Common::String &errorMessage) const;

	bool loadPacked(const SharedActorAssets &assets,
	                ActorPackedResourceHost &host,
	                ActorModel &out,
	                Common::String &errorMessage) const;
};

/**
 * Register the named actor objects in the Stage 6 scene registry.
 *
 * Geometry remains in ActorModel::model; this function supplies stable object
 * names for visibility, animation target resolution and picking.
 */
void registerActorSceneObjects(const ActorModel &actor, SceneRegistry &registry);

} // End of namespace ZeroComico

#endif
