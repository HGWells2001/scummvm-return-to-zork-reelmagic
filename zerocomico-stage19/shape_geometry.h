/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE19_SHAPE_GEOMETRY_H
#define ZEROCOMICO_STAGE19_SHAPE_GEOMETRY_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage16/shape_document.h"

namespace ZeroComico {

struct ShapeVec3 {
	float x;
	float y;
	float z;

	ShapeVec3() : x(0.0f), y(0.0f), z(0.0f) {}
	ShapeVec3(float px, float py, float pz) : x(px), y(py), z(pz) {}
};

struct ShapeEndpoint {
	uint32 lineNumber;
	Common::String label;
	Common::Array<Common::String> rawValues;
	bool numeric;
	ShapeVec3 value;

	ShapeEndpoint() : lineNumber(0), numeric(false) {}
};

struct ShapeGeometryRecord {
	uint32 lineNumber;
	Common::String name;
	ShapeDefinitionKind kind;
	Common::String rawType;
	ShapeEndpoint a;
	ShapeEndpoint b;
	bool hasA;
	bool hasB;

	ShapeGeometryRecord() :
		lineNumber(0),
		kind(kShapeDefinitionUnknown),
		hasA(false),
		hasB(false) {}

	bool hasNumericA() const { return hasA && a.numeric; }
	bool hasNumericB() const { return hasB && b.numeric; }
};

class ShapeGeometryDocument {
public:
	void clear();

	const ShapeGeometryRecord *shape(const Common::String &name) const;
	const Common::Array<ShapeGeometryRecord> &shapes() const { return _shapes; }

private:
	friend class ShapeGeometryParser;
	Common::Array<ShapeGeometryRecord> _shapes;
};

/**
 * Conservative body reader for Shape.shp.
 *
 * The retail grammar census proves that .shp uses ge_Shape/ge_Polygon plus
 * only A and B as body-leading keywords. Stage 19 attaches A/B records to the
 * most recent ge_Shape declaration and promotes them to ShapeVec3 only when
 * the line contains exactly three valid numeric tokens.
 *
 * It does not assign A/B semantics (position, direction, radius, etc.).
 */
class ShapeGeometryParser {
public:
	bool parse(const Common::String &decodedShapeScript,
	           ShapeGeometryDocument &out,
	           Common::String &errorMessage) const;
};

struct CharacterStartGeometry {
	Common::String characterName;
	Common::String helperName;
	const ShapeGeometryRecord *geometry;

	CharacterStartGeometry() : geometry(nullptr) {}

	bool valid() const {
		return geometry && geometry->kind == kShapeDefinitionPosition;
	}
};

bool resolveCharacterStartGeometry(
	const Common::String &characterName,
	const Common::String &helperName,
	const ShapeGeometryDocument &geometry,
	CharacterStartGeometry &out,
	Common::String &errorMessage);

} // End of namespace ZeroComico

#endif
