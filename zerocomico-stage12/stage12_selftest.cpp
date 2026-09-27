/* Zero Comico Stage 12 actor render-frame self-test. */

#include <cassert>
#include <cmath>

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage6/timeline_eval.h"
#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage9/actor_render_catalog.h"
#include "zerocomico-stage12/actor_render_frame.h"

using namespace ZeroComico;

static bool closeFloat(float a, float b) {
	return std::fabs(a - b) < 0.0005f;
}

static void assertVec(const Vec3f &v, float x, float y, float z) {
	assert(closeFloat(v.x, x));
	assert(closeFloat(v.y, y));
	assert(closeFloat(v.z, z));
}

static ActorModel makeActor() {
	ActorModel actor;
	actor.assets.name = "Giovanni";

	P3DMaterial material;
	material.name = "skin";
	material.textureResource = "GIO_SKIN.TGA";
	actor.model.materials.push_back(material);

	P3DMesh mesh;
	mesh.name = "gio_body";
	mesh.storage = kP3DMeshClassic;
	mesh.vertices.push_back(Vec3f(1.0f, 0.0f, 0.0f));
	mesh.vertices.push_back(Vec3f(0.0f, 1.0f, 0.0f));
	mesh.vertices.push_back(Vec3f(0.0f, 0.0f, 1.0f));

	P3DTriangle tri;
	tri.a = 0;
	tri.b = 1;
	tri.c = 2;
	mesh.triangles.push_back(tri);

	P3DMaterialGroup group;
	group.materialName = "skin";
	group.firstTriangle = 0;
	group.triangleCount = 1;
	mesh.materialGroups.push_back(group);

	P3DUV uv;
	uv.u = 0.0f; uv.v = 0.0f; mesh.triangleUVs.push_back(uv);
	uv.u = 1.0f; uv.v = 0.0f; mesh.triangleUVs.push_back(uv);
	uv.u = 0.0f; uv.v = 1.0f; mesh.triangleUVs.push_back(uv);

	actor.model.meshes.push_back(mesh);
	return actor;
}

static void testWorldSpaceFrame() {
	ActorModel actor = makeActor();

	ActorRenderCatalog catalog;
	Common::String error;
	assert(catalog.build(actor, error));
	assert(error.empty());
	assert(catalog.batches().size() == 1);

	SceneRegistry registry;
	registry.addObject("gio_body", kSceneObjectMesh);
	const int32 bodyId = registry.findObject("gio_body");
	SceneObjectState *body = registry.object(bodyId);
	assert(body);

	body->currentTransform.translation = Vec3f(10.0f, 0.0f, 0.0f);
	body->currentTransform.scale = Vec3f(2.0f, 2.0f, 2.0f);
	body->currentTransform.rotation =
		axisAngleToQuaternion(Vec3f(0.0f, 0.0f, 1.0f),
		                      (float)(M_PI * 0.5));

	TransformSample root;
	root.translation = Vec3f(0.0f, 5.0f, 0.0f);

	ActorRenderFrame frame;
	ActorRenderFrameBuilder builder;
	assert(builder.build(actor, catalog, registry, root, frame, error));
	assert(error.empty());
	assert(frame.visibleMeshes == 1);
	assert(frame.skippedInvisibleMeshes == 0);
	assert(frame.triangles.size() == 1);

	const ActorRenderTriangle &out = frame.triangles[0];
	assert(out.meshIndex == 0);
	assert(out.triangleIndex == 0);
	assert(out.materialIndex == 0);
	assert(out.materialName == "skin");
	assert(out.textureResource == "GIO_SKIN.TGA");

	// local (1,0,0) -> scale (2,0,0) -> rotate Z90 (0,2,0)
	// -> mesh translation (10,2,0) -> actor root (10,7,0)
	assertVec(out.vertex[0].position, 10.0f, 7.0f, 0.0f);

	// local (0,1,0) -> (0,2,0) -> (-2,0,0)
	// -> (8,0,0) -> root (8,5,0)
	assertVec(out.vertex[1].position, 8.0f, 5.0f, 0.0f);

	// local Z stays on Z under a Z-axis rotation.
	assertVec(out.vertex[2].position, 10.0f, 5.0f, 2.0f);

	assert(out.vertex[0].hasUV);
	assert(closeFloat(out.vertex[0].uv.u, 0.0f));
	assert(closeFloat(out.vertex[0].uv.v, 0.0f));
	assert(closeFloat(out.vertex[1].uv.u, 1.0f));
	assert(closeFloat(out.vertex[2].uv.v, 1.0f));
}

static void testVisibility() {
	ActorModel actor = makeActor();
	ActorRenderCatalog catalog;
	Common::String error;
	assert(catalog.build(actor, error));

	SceneRegistry registry;
	registry.addObject("gio_body", kSceneObjectMesh);
	registry.setVisible("gio_body", false);

	TransformSample root;
	ActorRenderFrame frame;
	ActorRenderFrameBuilder builder;
	assert(builder.build(actor, catalog, registry, root, frame, error));
	assert(frame.triangles.empty());
	assert(frame.visibleMeshes == 0);
	assert(frame.skippedInvisibleMeshes == 1);

	root.visible = false;
	registry.setVisible("gio_body", true);
	assert(builder.build(actor, catalog, registry, root, frame, error));
	assert(frame.triangles.empty());
	assert(frame.visibleMeshes == 0);
	assert(frame.skippedInvisibleMeshes == 0);
}

static void testMalformedUvTableRejected() {
	ActorModel actor = makeActor();
	actor.model.meshes[0].triangleUVs.pop_back();

	ActorRenderCatalog catalog;
	Common::String error;
	assert(catalog.build(actor, error));

	SceneRegistry registry;
	registry.addObject("gio_body", kSceneObjectMesh);

	TransformSample root;
	ActorRenderFrame frame;
	ActorRenderFrameBuilder builder;
	assert(!builder.build(actor, catalog, registry, root, frame, error));
	assert(!error.empty());
}

int main() {
	testWorldSpaceFrame();
	testVisibility();
	testMalformedUvTableRejected();
	return 0;
}
