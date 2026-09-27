/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_P3D_MODEL_H
#define ZEROCOMICO_STAGE9_P3D_MODEL_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "zerocomico-stage6/timeline_eval.h"

namespace ZeroComico {

struct P3DTriangle {
	uint16 a;
	uint16 b;
	uint16 c;
};

struct P3DUV {
	float u;
	float v;
};

struct P3DMaterialGroup {
	Common::String materialName;
	uint32 firstTriangle;
	uint32 triangleCount;
};

struct P3DMaterial {
	Common::String name;
	Common::String textureResource;
	Common::Array<byte> rawBody;
};

struct P3DCamera {
	Common::String name;
	Vec3f source;
	Vec3f target;
	float roll;
	float fov;
	float rangeNear;
	float rangeFar;
};

struct P3DLight {
	Common::String name;
	uint32 flags;
	Vec3f vectorA;
	Vec3f vectorB;
	Vec3f vectorC;
	uint32 scalar[5];
	Common::Array<Common::String> linkedNames;
};

enum P3DMeshStorage {
	kP3DMeshClassic,
	kP3DMeshSharedVertices,
	kP3DMeshDeformer
};

struct P3DDeformerVertex {
	Vec3f position;
	uint32 aux0;
	uint32 aux1;
};

struct P3DMesh {
	Common::String name;
	uint32 flags;
	P3DMeshStorage storage;

	Vec3f headerVector;
	float matrix[9];
	Vec3f vectorB;
	Vec3f vectorC;

	Common::String referenceName;
	Common::Array<Vec3f> vertices;
	Common::Array<P3DDeformerVertex> deformerVertices;
	Vec3f postVertexVector;

	Common::Array<P3DTriangle> triangles;
	Common::Array<P3DMaterialGroup> materialGroups;
	Common::Array<P3DUV> triangleUVs;

	// Structurally decoded optional 0x40 payload. Its original semantic name
	// is intentionally not guessed.
	Common::Array<Vec3f> extraVectors;
	Common::Array<byte> extraVertexBytes;
	Common::Array<P3DTriangle> extraTriangles;

	P3DMesh();
};

struct P3DModel {
	uint16 version;
	Common::Array<P3DMaterial> materials;
	Common::Array<P3DCamera> cameras;
	Common::Array<P3DLight> lights;
	Common::Array<P3DMesh> meshes;

	void clear();
};

class P3DModelParser {
public:
	bool parse(const Common::Array<byte> &decoded,
	           P3DModel &out,
	           Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
