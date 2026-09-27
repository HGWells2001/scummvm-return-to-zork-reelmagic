/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cstring>

#include "common/endian.h"

#include "zerocomico-stage9/p3d_model.h"

namespace ZeroComico {

namespace {

static const uint16 kRecordMark = 0xAABB;
static const uint16 kFileMagic = 0x0E3D;
static const uint16 kMaterialRecord = 0xF000;
static const uint16 kCameraRecord = 0xF001;
static const uint16 kLightRecord = 0xF002;
static const uint16 kMeshRecord = 0xF003;

class Cursor {
public:
	Cursor(const byte *data, uint32 size) : _data(data), _size(size), _pos(0) {}

	uint32 pos() const { return _pos; }
	uint32 size() const { return _size; }
	uint32 remaining() const { return _pos <= _size ? _size - _pos : 0; }

	bool seek(uint32 pos) {
		if (pos > _size)
			return false;
		_pos = pos;
		return true;
	}

	bool skip(uint32 count) {
		return seek(_pos + count);
	}

	bool readU8(byte &v) {
		if (remaining() < 1)
			return false;
		v = _data[_pos++];
		return true;
	}

	bool readU16(uint16 &v) {
		if (remaining() < 2)
			return false;
		v = READ_LE_UINT16(_data + _pos);
		_pos += 2;
		return true;
	}

	bool readU32(uint32 &v) {
		if (remaining() < 4)
			return false;
		v = READ_LE_UINT32(_data + _pos);
		_pos += 4;
		return true;
	}

	bool readFloat(float &v) {
		if (remaining() < 4)
			return false;
		v = READ_LE_FLOAT32(_data + _pos);
		_pos += 4;
		return true;
	}

	bool readBytes(byte *dst, uint32 count) {
		if (remaining() < count)
			return false;
		memcpy(dst, _data + _pos, count);
		_pos += count;
		return true;
	}

	bool copyBytes(Common::Array<byte> &dst, uint32 count) {
		if (remaining() < count)
			return false;
		dst.resize(count);
		if (count)
			memcpy(dst.data(), _data + _pos, count);
		_pos += count;
		return true;
	}

	bool hasBytes(const byte *bytes, uint32 count) const {
		return remaining() >= count && memcmp(_data + _pos, bytes, count) == 0;
	}

private:
	const byte *_data;
	uint32 _size;
	uint32 _pos;
};

static bool readVec3(Cursor &c, Vec3f &v) {
	return c.readFloat(v.x) && c.readFloat(v.y) && c.readFloat(v.z);
}

static bool readName32(Cursor &c, Common::String &name) {
	byte raw[32];
	if (!c.readBytes(raw, sizeof(raw)))
		return false;

	uint32 length = 0;
	while (length < sizeof(raw) && raw[length] != 0)
		++length;
	name = Common::String((const char *)raw, length);
	return true;
}

static bool readTriangle(Cursor &c, P3DTriangle &t) {
	return c.readU16(t.a) && c.readU16(t.b) && c.readU16(t.c);
}

static bool consumeRecordEnd(Cursor &c) {
	static const byte kEnd[3] = {0xED, 0xFF, 0xFF};
	if (!c.hasBytes(kEnd, sizeof(kEnd)))
		return false;
	return c.skip(sizeof(kEnd));
}

static bool atFileTrailer(const Cursor &c) {
	static const byte kTrailer[4] = {0x00, 0xED, 0xFF, 0xFF};
	return c.hasBytes(kTrailer, sizeof(kTrailer));
}

static bool parseMaterial(Cursor &c, const Common::String &name,
                          P3DMaterial &out, Common::String &error) {
	const uint32 bodyStart = c.pos();

	// The retail corpus has only 48-byte and 92-byte F000 bodies.
	if (c.remaining() < 48) {
		error = Common::String::format("Material '%s' is truncated", name.c_str());
		return false;
	}

	if (!c.copyBytes(out.rawBody, 48))
		return false;
	out.name = name;

	static const byte kEnd[3] = {0xED, 0xFF, 0xFF};
	if (c.hasBytes(kEnd, sizeof(kEnd)))
		return true;

	// 92-byte variant: resource field begins at body +48.
	if (c.remaining() < 44) {
		error = Common::String::format("Material '%s' has unknown body size", name.c_str());
		return false;
	}

	byte extra[44];
	if (!c.readBytes(extra, sizeof(extra)))
		return false;

	for (uint32 i = 0; i < sizeof(extra); ++i)
		out.rawBody.push_back(extra[i]);

	uint32 resourceLength = 0;
	while (resourceLength < 32 && extra[resourceLength] != 0)
		++resourceLength;
	out.textureResource = Common::String((const char *)extra, resourceLength);

	if (c.pos() - bodyStart != 92) {
		error = "Internal material-size mismatch";
		return false;
	}
	return true;
}

static bool parseCamera(Cursor &c, const Common::String &name,
                        P3DCamera &out, Common::String &error) {
	out.name = name;
	uint32 marker = 0;
	if (!readVec3(c, out.source) ||
	    !readVec3(c, out.target) ||
	    !c.readFloat(out.roll) ||
	    !c.readFloat(out.fov) ||
	    !c.readU32(marker) ||
	    !c.readFloat(out.rangeNear) ||
	    !c.readFloat(out.rangeFar)) {
		error = Common::String::format("Camera '%s' is truncated", name.c_str());
		return false;
	}
	if (marker != 0xF0F01234) {
		error = Common::String::format("Camera '%s' has invalid range marker", name.c_str());
		return false;
	}
	return true;
}

static bool parseLight(Cursor &c, const Common::String &name,
                       P3DLight &out, Common::String &error) {
	out.name = name;
	if (!c.readU32(out.flags) ||
	    !readVec3(c, out.vectorA) ||
	    !readVec3(c, out.vectorB) ||
	    !readVec3(c, out.vectorC)) {
		error = Common::String::format("Light '%s' is truncated", name.c_str());
		return false;
	}

	for (uint32 i = 0; i < 5; ++i) {
		if (!c.readU32(out.scalar[i])) {
			error = Common::String::format("Light '%s' is truncated", name.c_str());
			return false;
		}
	}

	if (out.flags & 0x100) {
		uint32 count = 0;
		if (!c.readU32(count) || count > 4096) {
			error = Common::String::format("Light '%s' has invalid linked-name count", name.c_str());
			return false;
		}
		for (uint32 i = 0; i < count; ++i) {
			Common::String linked;
			if (!readName32(c, linked)) {
				error = Common::String::format("Light '%s' linked-name table is truncated", name.c_str());
				return false;
			}
			out.linkedNames.push_back(linked);
		}
	}
	return true;
}

static bool parseMeshCommon(Cursor &c, P3DMesh &mesh, Common::String &error) {
	if (!c.readU32(mesh.flags) || !readVec3(c, mesh.headerVector)) {
		error = Common::String::format("Mesh '%s' has truncated header", mesh.name.c_str());
		return false;
	}

	for (uint32 i = 0; i < 9; ++i) {
		if (!c.readFloat(mesh.matrix[i])) {
			error = Common::String::format("Mesh '%s' has truncated matrix", mesh.name.c_str());
			return false;
		}
	}

	if (!readVec3(c, mesh.vectorB) || !readVec3(c, mesh.vectorC)) {
		error = Common::String::format("Mesh '%s' has truncated vectors", mesh.name.c_str());
		return false;
	}
	return true;
}

static bool parseMeshClassicOrShared(Cursor &c, P3DMesh &mesh,
                                     Common::String &error) {
	uint32 vertexCount = 0;
	uint32 triangleCount = 0;
	uint32 materialCount = 0;

	if (!c.readU32(vertexCount) ||
	    !c.readU32(triangleCount) ||
	    !c.readU32(materialCount)) {
		error = Common::String::format("Mesh '%s' has truncated counts", mesh.name.c_str());
		return false;
	}

	if (vertexCount > 1000000 || triangleCount > 1000000 || materialCount > 100000) {
		error = Common::String::format("Mesh '%s' has unreasonable counts", mesh.name.c_str());
		return false;
	}

	mesh.storage = (mesh.flags & 0x10000)
		? kP3DMeshSharedVertices : kP3DMeshClassic;

	if (mesh.storage == kP3DMeshClassic) {
		mesh.vertices.resize(vertexCount);
		for (uint32 i = 0; i < vertexCount; ++i) {
			Vec3f raw;
			if (!readVec3(c, raw)) {
				error = Common::String::format("Mesh '%s' vertex table is truncated", mesh.name.c_str());
				return false;
			}
			// Exact operation recovered from japotek3d.dll.
			mesh.vertices[i] = raw + mesh.vectorB - mesh.headerVector;
		}
	}

	if (!readVec3(c, mesh.postVertexVector)) {
		error = Common::String::format("Mesh '%s' post-vertex vector is truncated", mesh.name.c_str());
		return false;
	}

	mesh.triangles.resize(triangleCount);
	for (uint32 i = 0; i < triangleCount; ++i) {
		if (!readTriangle(c, mesh.triangles[i])) {
			error = Common::String::format("Mesh '%s' triangle table is truncated", mesh.name.c_str());
			return false;
		}
		if (mesh.storage == kP3DMeshClassic &&
		    (mesh.triangles[i].a >= vertexCount ||
		     mesh.triangles[i].b >= vertexCount ||
		     mesh.triangles[i].c >= vertexCount)) {
			error = Common::String::format("Mesh '%s' triangle index is out of range", mesh.name.c_str());
			return false;
		}
	}

	mesh.materialGroups.resize(materialCount);
	for (uint32 i = 0; i < materialCount; ++i) {
		P3DMaterialGroup &group = mesh.materialGroups[i];
		if (!readName32(c, group.materialName) ||
		    !c.readU32(group.firstTriangle) ||
		    !c.readU32(group.triangleCount)) {
			error = Common::String::format("Mesh '%s' material table is truncated", mesh.name.c_str());
			return false;
		}
		if (group.firstTriangle > triangleCount ||
		    group.triangleCount > triangleCount - group.firstTriangle) {
			error = Common::String::format("Mesh '%s' material range is invalid", mesh.name.c_str());
			return false;
		}
	}

	if (mesh.flags & 0x20) {
		const uint32 uvCount = triangleCount * 3;
		mesh.triangleUVs.resize(uvCount);
		for (uint32 i = 0; i < uvCount; ++i) {
			if (!c.readFloat(mesh.triangleUVs[i].u) ||
			    !c.readFloat(mesh.triangleUVs[i].v)) {
				error = Common::String::format("Mesh '%s' UV table is truncated", mesh.name.c_str());
				return false;
			}
		}
	}

	if (mesh.flags & 0x40) {
		uint32 extraCount = 0;
		if (!c.readU32(extraCount) || extraCount > 1000000) {
			error = Common::String::format("Mesh '%s' has invalid auxiliary count", mesh.name.c_str());
			return false;
		}

		mesh.extraVectors.resize(extraCount);
		for (uint32 i = 0; i < extraCount; ++i) {
			if (!readVec3(c, mesh.extraVectors[i])) {
				error = Common::String::format("Mesh '%s' auxiliary vectors are truncated", mesh.name.c_str());
				return false;
			}
		}

		mesh.extraVertexBytes.resize(vertexCount);
		for (uint32 i = 0; i < vertexCount; ++i) {
			if (!c.readU8(mesh.extraVertexBytes[i])) {
				error = Common::String::format("Mesh '%s' auxiliary vertex table is truncated", mesh.name.c_str());
				return false;
			}
		}

		mesh.extraTriangles.resize(triangleCount);
		for (uint32 i = 0; i < triangleCount; ++i) {
			if (!readTriangle(c, mesh.extraTriangles[i])) {
				error = Common::String::format("Mesh '%s' auxiliary triangle table is truncated", mesh.name.c_str());
				return false;
			}
		}
	}

	return true;
}

static bool parseMeshDeformer(Cursor &c, P3DMesh &mesh, Common::String &error) {
	mesh.storage = kP3DMeshDeformer;

	if (!readName32(c, mesh.referenceName)) {
		error = Common::String::format("Mesh '%s' is missing deformer reference", mesh.name.c_str());
		return false;
	}

	uint32 vertexCount = 0;
	if (!c.readU32(vertexCount) || vertexCount > 1000000) {
		error = Common::String::format("Mesh '%s' has invalid deformer vertex count", mesh.name.c_str());
		return false;
	}

	mesh.deformerVertices.resize(vertexCount);
	for (uint32 i = 0; i < vertexCount; ++i) {
		if (!readVec3(c, mesh.deformerVertices[i].position) ||
		    !c.readU32(mesh.deformerVertices[i].aux0) ||
		    !c.readU32(mesh.deformerVertices[i].aux1)) {
			error = Common::String::format("Mesh '%s' deformer vertex table is truncated", mesh.name.c_str());
			return false;
		}
	}

	if (!readVec3(c, mesh.postVertexVector)) {
		error = Common::String::format("Mesh '%s' deformer tail is truncated", mesh.name.c_str());
		return false;
	}
	return true;
}

static bool parseMesh(Cursor &c, const Common::String &name,
                      P3DMesh &mesh, Common::String &error) {
	mesh.name = name;
	if (!parseMeshCommon(c, mesh, error))
		return false;

	if (mesh.flags & 0x80000000) {
		error = Common::String::format("Mesh '%s' uses unsupported 0x80000000 branch", name.c_str());
		return false;
	}

	if (mesh.flags & 0x20000)
		return parseMeshDeformer(c, mesh, error);

	return parseMeshClassicOrShared(c, mesh, error);
}

} // namespace

P3DMesh::P3DMesh() :
	flags(0),
	storage(kP3DMeshClassic),
	headerVector(),
	vectorB(),
	vectorC(),
	postVertexVector() {
	for (uint32 i = 0; i < ARRAYSIZE(matrix); ++i)
		matrix[i] = 0.0f;
}

void P3DModel::clear() {
	version = 0;
	materials.clear();
	cameras.clear();
	lights.clear();
	meshes.clear();
}

bool P3DModelParser::parse(const Common::Array<byte> &decoded,
                           P3DModel &out,
                           Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decoded.size() < 8) {
		errorMessage = "P3D is too small";
		return false;
	}

	Cursor c(decoded.data(), decoded.size());
	uint16 version = 0;
	uint16 magic = 0;
	if (!c.readU16(version) || !c.readU16(magic) || magic != kFileMagic) {
		errorMessage = "Invalid P3D header";
		return false;
	}
	if (version != 1 && version != 2) {
		errorMessage = Common::String::format("Unsupported P3D version %u", version);
		return false;
	}
	out.version = version;

	while (!atFileTrailer(c)) {
		uint16 mark = 0;
		uint16 type = 0;
		if (!c.readU16(mark) || !c.readU16(type)) {
			errorMessage = "Truncated P3D record header";
			return false;
		}
		if (mark != kRecordMark) {
			errorMessage = Common::String::format("Invalid P3D record mark at %u", c.pos() - 4);
			return false;
		}

		Common::String name;
		if (!readName32(c, name)) {
			errorMessage = "Truncated P3D record name";
			return false;
		}

		bool ok = false;
		switch (type) {
		case kMaterialRecord: {
			P3DMaterial material;
			ok = parseMaterial(c, name, material, errorMessage);
			if (ok)
				out.materials.push_back(material);
			break;
		}
		case kCameraRecord: {
			P3DCamera camera;
			ok = parseCamera(c, name, camera, errorMessage);
			if (ok)
				out.cameras.push_back(camera);
			break;
		}
		case kLightRecord: {
			P3DLight light;
			ok = parseLight(c, name, light, errorMessage);
			if (ok)
				out.lights.push_back(light);
			break;
		}
		case kMeshRecord: {
			P3DMesh mesh;
			ok = parseMesh(c, name, mesh, errorMessage);
			if (ok)
				out.meshes.push_back(mesh);
			break;
		}
		default:
			errorMessage = Common::String::format("Unexpected P3D record type 0x%04X", type);
			return false;
		}

		if (!ok)
			return false;
		if (!consumeRecordEnd(c)) {
			errorMessage = Common::String::format(
				"Record '%s' type 0x%04X did not end at the proven boundary",
				name.c_str(), type);
			return false;
		}
	}

	static const byte kTrailer[4] = {0x00, 0xED, 0xFF, 0xFF};
	if (!c.hasBytes(kTrailer, sizeof(kTrailer)) || !c.skip(sizeof(kTrailer)) ||
	    c.pos() != c.size()) {
		errorMessage = "Invalid P3D file trailer";
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
