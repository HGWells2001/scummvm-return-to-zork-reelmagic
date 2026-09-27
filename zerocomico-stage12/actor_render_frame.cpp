/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cmath>

#include "zerocomico-stage12/actor_render_frame.h"

namespace ZeroComico {

namespace {

static Quatf normalizedQuaternion(const Quatf &q) {
	const float lengthSquared =
		q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lengthSquared <= 0.0000001f)
		return Quatf();

	const float invLength = 1.0f / std::sqrt(lengthSquared);
	return Quatf(q.x * invLength, q.y * invLength,
	             q.z * invLength, q.w * invLength);
}

static Vec3f crossProduct(const Vec3f &a, const Vec3f &b) {
	return Vec3f(a.y * b.z - a.z * b.y,
	             a.z * b.x - a.x * b.z,
	             a.x * b.y - a.y * b.x);
}

static Vec3f rotateByQuaternion(const Quatf &rotation,
                                const Vec3f &point) {
	const Quatf q = normalizedQuaternion(rotation);
	const Vec3f axis(q.x, q.y, q.z);
	const Vec3f doubledCross = crossProduct(axis, point) * 2.0f;
	return point + doubledCross * q.w + crossProduct(axis, doubledCross);
}

static bool validateUvTable(const P3DMesh &mesh,
                            Common::String &errorMessage) {
	if (mesh.triangleUVs.empty())
		return true;

	const uint32 expected = mesh.triangles.size() * 3;
	if (mesh.triangleUVs.size() != expected) {
		errorMessage = Common::String::format(
			"Mesh '%s' has %u UV entries, expected %u",
			mesh.name.c_str(), (uint)mesh.triangleUVs.size(), (uint)expected);
		return false;
	}
	return true;
}

} // namespace

Vec3f applyActorTransform(const TransformSample &transform,
                          const Vec3f &point) {
	Vec3f scaled(point.x * transform.scale.x,
	             point.y * transform.scale.y,
	             point.z * transform.scale.z);
	return rotateByQuaternion(transform.rotation, scaled) +
	       transform.translation;
}

bool ActorRenderFrameBuilder::build(const ActorModel &actor,
                                    const ActorRenderCatalog &catalog,
                                    const SceneRegistry &registry,
                                    const TransformSample &actorRoot,
                                    ActorRenderFrame &out,
                                    Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (!actorRoot.visible)
		return true;

	Common::Array<bool> countedMeshes;
	countedMeshes.resize(actor.model.meshes.size());
	for (uint32 i = 0; i < countedMeshes.size(); ++i)
		countedMeshes[i] = false;

	for (uint32 b = 0; b < catalog.batches().size(); ++b) {
		const ActorRenderBatch &batch = catalog.batches()[b];
		if (batch.meshIndex >= actor.model.meshes.size()) {
			errorMessage = "Actor render batch references an invalid mesh";
			return false;
		}

		const P3DMesh &mesh = actor.model.meshes[batch.meshIndex];
		if (mesh.storage != kP3DMeshClassic) {
			errorMessage = Common::String::format(
				"Render batch references non-classic mesh '%s'",
				mesh.name.c_str());
			return false;
		}

		const int32 sceneObjectId = registry.findObject(mesh.name);
		const SceneObjectState *sceneObject = registry.object(sceneObjectId);
		if (!sceneObject) {
			errorMessage = Common::String::format(
				"Renderable mesh '%s' is not registered in the scene",
				mesh.name.c_str());
			return false;
		}

		if (!validateUvTable(mesh, errorMessage))
			return false;

		if (!sceneObject->visible()) {
			if (!countedMeshes[batch.meshIndex]) {
				++out.skippedInvisibleMeshes;
				countedMeshes[batch.meshIndex] = true;
			}
			continue;
		}

		if (!countedMeshes[batch.meshIndex]) {
			++out.visibleMeshes;
			countedMeshes[batch.meshIndex] = true;
		}

		if (batch.firstTriangle > mesh.triangles.size() ||
		    batch.triangleCount > mesh.triangles.size() - batch.firstTriangle) {
			errorMessage = Common::String::format(
				"Render batch range is invalid for mesh '%s'",
				mesh.name.c_str());
			return false;
		}

		for (uint32 n = 0; n < batch.triangleCount; ++n) {
			const uint32 triangleIndex = batch.firstTriangle + n;
			const P3DTriangle &source = mesh.triangles[triangleIndex];
			const uint16 index[3] = {source.a, source.b, source.c};

			ActorRenderTriangle triangle;
			triangle.meshIndex = batch.meshIndex;
			triangle.triangleIndex = triangleIndex;
			triangle.materialIndex = batch.materialIndex;
			triangle.materialName = batch.materialName;
			triangle.textureResource = batch.textureResource;

			for (uint32 corner = 0; corner < 3; ++corner) {
				if (index[corner] >= mesh.vertices.size()) {
					errorMessage = Common::String::format(
						"Mesh '%s' triangle %u references vertex %u of %u",
						mesh.name.c_str(), (uint)triangleIndex,
						(uint)index[corner], (uint)mesh.vertices.size());
					return false;
				}

				const Vec3f animated =
					applyActorTransform(sceneObject->currentTransform,
					                    mesh.vertices[index[corner]]);
				triangle.vertex[corner].position =
					applyActorTransform(actorRoot, animated);

				if (!mesh.triangleUVs.empty()) {
					triangle.vertex[corner].uv =
						mesh.triangleUVs[triangleIndex * 3 + corner];
					triangle.vertex[corner].hasUV = true;
				}
			}

			out.triangles.push_back(triangle);
		}
	}

	return true;
}

} // End of namespace ZeroComico
