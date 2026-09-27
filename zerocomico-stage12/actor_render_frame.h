/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE12_ACTOR_RENDER_FRAME_H
#define ZEROCOMICO_STAGE12_ACTOR_RENDER_FRAME_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage9/actor_render_catalog.h"

namespace ZeroComico {

struct ActorRenderVertex {
	Vec3f position;
	P3DUV uv;
	bool hasUV;

	ActorRenderVertex() : hasUV(false) {}
};

struct ActorRenderTriangle {
	ActorRenderVertex vertex[3];
	uint32 meshIndex;
	uint32 triangleIndex;
	int32 materialIndex;
	Common::String materialName;
	Common::String textureResource;

	ActorRenderTriangle() :
		meshIndex(0), triangleIndex(0), materialIndex(-1) {}
};

struct ActorRenderFrame {
	Common::Array<ActorRenderTriangle> triangles;
	uint32 visibleMeshes;
	uint32 skippedInvisibleMeshes;

	ActorRenderFrame() :
		visibleMeshes(0), skippedInvisibleMeshes(0) {}

	void clear() {
		triangles.clear();
		visibleMeshes = 0;
		skippedInvisibleMeshes = 0;
	}
};

/**
 * Builds world-space triangles for the proven classic F003 meshes.
 *
 * Deliberately excluded:
 * - the preserved but semantically unproven P3D mesh.matrix;
 * - camera projection / handedness;
 * - shared/deformer mesh ownership.
 *
 * Those boundaries stay explicit instead of being guessed.
 */
class ActorRenderFrameBuilder {
public:
	bool build(const ActorModel &actor,
	           const ActorRenderCatalog &catalog,
	           const SceneRegistry &registry,
	           const TransformSample &actorRoot,
	           ActorRenderFrame &out,
	           Common::String &errorMessage) const;
};

Vec3f applyActorTransform(const TransformSample &transform,
                          const Vec3f &point);

} // End of namespace ZeroComico

#endif
