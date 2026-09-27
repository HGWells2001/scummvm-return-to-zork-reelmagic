/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage9/actor_render_catalog.h"

namespace ZeroComico {

namespace {

static int32 findMaterial(const P3DModel &model, const Common::String &name) {
	for (uint32 i = 0; i < model.materials.size(); ++i) {
		if (model.materials[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

} // namespace

bool ActorRenderCatalog::build(const ActorModel &actor,
                               Common::String &errorMessage) {
	_renderableMeshes.clear();
	_batches.clear();
	errorMessage.clear();

	for (uint32 meshIndex = 0; meshIndex < actor.model.meshes.size(); ++meshIndex) {
		const P3DMesh &mesh = actor.model.meshes[meshIndex];

		// F003 classic records own both positions and indexed triangles.
		// Shared/deformer records are preserved by the model loader but need
		// their ownership/deformation relationship resolved before rasterizing.
		if (mesh.storage != kP3DMeshClassic ||
		    mesh.vertices.empty() ||
		    mesh.triangles.empty())
			continue;

		_renderableMeshes.push_back(meshIndex);

		if (mesh.materialGroups.empty()) {
			ActorRenderBatch batch;
			batch.meshIndex = meshIndex;
			batch.firstTriangle = 0;
			batch.triangleCount = mesh.triangles.size();
			_batches.push_back(batch);
			continue;
		}

		for (uint32 g = 0; g < mesh.materialGroups.size(); ++g) {
			const P3DMaterialGroup &group = mesh.materialGroups[g];
			ActorRenderBatch batch;
			batch.meshIndex = meshIndex;
			batch.firstTriangle = group.firstTriangle;
			batch.triangleCount = group.triangleCount;
			batch.materialName = group.materialName;
			batch.materialIndex = findMaterial(actor.model, group.materialName);
			if (batch.materialIndex >= 0)
				batch.textureResource =
					actor.model.materials[batch.materialIndex].textureResource;
			_batches.push_back(batch);
		}
	}

	if (_renderableMeshes.empty()) {
		errorMessage = Common::String::format(
			"Actor '%s' has no classic renderable F003 mesh",
			actor.assets.name.c_str());
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
