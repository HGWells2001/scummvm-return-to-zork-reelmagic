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

namespace ZeroComico {

class ActorBinaryResourceHost {
public:
	virtual ~ActorBinaryResourceHost() {}

	/**
	 * Read a resource after opening its JFX1 wrapper and LZHUF decoding it.
	 * The Stage 1 decoder is the intended implementation.
	 */
	virtual bool readDecodedBinary(const Common::Path &path,
	                               Common::Array<byte> &decoded) = 0;
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
