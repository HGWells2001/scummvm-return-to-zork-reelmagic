/* Zero Comico Stage 9 focused parser self-test.
 *
 * Syntax-checked in CI. It can also be linked into an engine-side test
 * harness to exercise the deterministic P3D -> ANJ -> AnimationClip path.
 */

#include <cassert>
#include <cstring>

#include "common/endian.h"

#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage9/actor_render_catalog.h"
#include "zerocomico-stage9/anj_tracks.h"
#include "zerocomico-stage9/p3d_model.h"

using namespace ZeroComico;

namespace {

static void putU16(Common::Array<byte> &out, uint16 v) {
	const uint32 p = out.size();
	out.resize(p + 2);
	WRITE_LE_UINT16(out.data() + p, v);
}

static void putU32(Common::Array<byte> &out, uint32 v) {
	const uint32 p = out.size();
	out.resize(p + 4);
	WRITE_LE_UINT32(out.data() + p, v);
}

static void putFloat(Common::Array<byte> &out, float v) {
	const uint32 p = out.size();
	out.resize(p + 4);
	WRITE_LE_FLOAT32(out.data() + p, v);
}

static void putName32(Common::Array<byte> &out, const char *name) {
	const uint32 p = out.size();
	out.resize(p + 32);
	memset(out.data() + p, '0', 32);
	const uint32 n = MIN<uint32>((uint32)strlen(name), 31);
	memcpy(out.data() + p, name, n);
	out[p + n] = 0;
}

static void putVec3(Common::Array<byte> &out, float x, float y, float z) {
	putFloat(out, x);
	putFloat(out, y);
	putFloat(out, z);
}

static void putEnd(Common::Array<byte> &out) {
	out.push_back(0xED);
	out.push_back(0xFF);
	out.push_back(0xFF);
}

static void putTrailer(Common::Array<byte> &out) {
	out.push_back(0x00);
	putEnd(out);
}

static void putNamedHeader(Common::Array<byte> &out, uint16 type, const char *name) {
	putU16(out, 0xAABB);
	putU16(out, type);
	putName32(out, name);
}

static Common::Array<byte> makeP3D() {
	Common::Array<byte> out;
	putU16(out, 2);
	putU16(out, 0x0E3D);

	// One 48-byte untextured material.
	putNamedHeader(out, 0xF000, "body_mat");
	for (uint32 i = 0; i < 48; ++i)
		out.push_back(0);
	putEnd(out);

	// One camera, using the proven fixed 44-byte body.
	putNamedHeader(out, 0xF001, "Camera01");
	putVec3(out, 10.0f, 0.0f, 0.0f);
	putVec3(out, 0.0f, 0.0f, 0.0f);
	putFloat(out, 0.0f);
	putFloat(out, 15.0f);
	putU32(out, 0xF0F01234);
	putFloat(out, 0.0f);
	putFloat(out, 1000.0f);
	putEnd(out);

	// One classic triangle mesh.
	putNamedHeader(out, 0xF003, "body");
	putU32(out, 0);                  // flags
	putVec3(out, 0, 0, 0);          // headerVector
	for (uint32 i = 0; i < 9; ++i)
		putFloat(out, (i == 0 || i == 4 || i == 8) ? 1.0f : 0.0f);
	putVec3(out, 0, 0, 0);          // vectorB
	putVec3(out, 0, 0, 0);          // vectorC
	putU32(out, 3);                  // vertices
	putU32(out, 1);                  // triangles
	putU32(out, 0);                  // material groups
	putVec3(out, 0, 0, 0);
	putVec3(out, 1, 0, 0);
	putVec3(out, 0, 1, 0);
	putVec3(out, 0, 0, 0);          // postVertexVector
	putU16(out, 0);
	putU16(out, 1);
	putU16(out, 2);
	putEnd(out);

	putTrailer(out);
	return out;
}

static void putVec3KeyChannel(Common::Array<byte> &out,
                              uint32 frame, float x, float y, float z) {
	putU32(out, 1);
	putU32(out, frame);
	putFloat(out, 0.0f); // tension
	putFloat(out, 0.0f); // continuity
	putFloat(out, 0.0f); // bias
	putVec3(out, x, y, z);
}

static void putRotationKeyChannel(Common::Array<byte> &out, uint32 frame) {
	putU32(out, 1);
	putU32(out, frame);
	putFloat(out, 0.0f);
	putFloat(out, 0.0f);
	putFloat(out, 0.0f);
	putVec3(out, 0.0f, 0.0f, 1.0f);
	putFloat(out, 0.0f);
}

static void putVisibilityChannel(Common::Array<byte> &out,
                                 uint32 frame, bool visible) {
	putU32(out, 1);
	putU32(out, frame);
	putU32(out, visible ? 1 : 0);
}

static Common::Array<byte> makeANJ() {
	Common::Array<byte> child;

	// Binding: flags + objectName + parentName = 68 bytes.
	putNamedHeader(child, 0xF003, "body");
	putU32(child, 0);
	putName32(child, "body");
	putName32(child, "root");
	putEnd(child);

	// One transform timeline.
	putNamedHeader(child, 0xF007, "body_walk");
	putName32(child, "Walk");
	putU32(child, 10);               // duration
	putU32(child, 0xF0F01234);
	putU32(child, 0);                // first frame
	putU32(child, 10);               // last frame
	putName32(child, "body");
	putVec3KeyChannel(child, 0, 0, 0, 0);
	putVec3KeyChannel(child, 0, 1, 1, 1);
	putRotationKeyChannel(child, 0);
	putVisibilityChannel(child, 0, true);
	putEnd(child);

	Common::Array<byte> out;
	putU16(out, 2);
	putU16(out, 0x0E3D);

	putU16(out, 0xAABB);
	putU16(out, 0xF044);
	putU32(out, child.size() + 4);
	for (uint32 i = 0; i < child.size(); ++i)
		out.push_back(child[i]);

	putTrailer(out);
	return out;
}

} // namespace

int main() {
	P3DModel model;
	P3DModelParser p3d;
	Common::String error;
	assert(p3d.parse(makeP3D(), model, error));
	assert(model.materials.size() == 1);
	assert(model.cameras.size() == 1);
	assert(model.meshes.size() == 1);
	assert(model.meshes[0].vertices.size() == 3);
	assert(model.meshes[0].triangles.size() == 1);

	ANJDocument document;
	ANJDocumentParser anj;
	assert(anj.parse(makeANJ(), document, error));
	assert(document.bindings.size() == 1);
	assert(document.timelines.size() == 1);
	assert(document.timelines[0].animationName == "Walk");

	Common::Array<AnimationClip> clips;
	ANJDecodeStats stats;
	ANJTrackDecoder tracks;
	assert(tracks.decode(model, document, clips, stats, error));
	assert(clips.size() == 1);
	assert(clips[0].name == "Walk");
	assert(clips[0].tracks.size() == 1);
	assert(clips[0].tracks[0].translationKeys.size() == 1);
	assert(stats.transformTargets == 1);

	ActorModel actor;
	actor.assets = makeSharedActorAssets("Giovanni");
	actor.model = model;
	actor.animationDocument = document;
	actor.animationClips = clips;
	actor.animationStats = stats;

	ActorRenderCatalog renderCatalog;
	assert(renderCatalog.build(actor, error));
	assert(renderCatalog.renderableMeshes().size() == 1);
	return 0;
}
