/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE16_SHAPE_DOCUMENT_H
#define ZEROCOMICO_STAGE16_SHAPE_DOCUMENT_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

enum ShapeDefinitionKind {
	kShapeDefinitionUnknown,
	kShapeDefinitionPosition,
	kShapeDefinitionPortal,
	kShapeDefinitionRange,
	kShapeDefinitionEntity
};

struct ShapeDefinition {
	uint32 lineNumber;
	Common::String name;
	Common::String rawType;
	ShapeDefinitionKind kind;
	Common::Array<Common::String> declarationTail;

	ShapeDefinition() :
		lineNumber(0),
		kind(kShapeDefinitionUnknown) {}
};

struct VectorDefinition {
	uint32 lineNumber;
	Common::String name;
	Common::Array<Common::String> declarationTail;

	VectorDefinition() : lineNumber(0) {}
};

class ShapeScriptDocument {
public:
	void clear();

	const ShapeDefinition *shape(const Common::String &name) const;
	const VectorDefinition *vector(const Common::String &name) const;

	const Common::Array<ShapeDefinition> &shapes() const { return _shapes; }
	const Common::Array<VectorDefinition> &vectors() const { return _vectors; }

private:
	friend class ShapeScriptParser;
	Common::Array<ShapeDefinition> _shapes;
	Common::Array<VectorDefinition> _vectors;
};

/**
 * Conservative declaration parser for Shape.shp.
 *
 * Proven top-level constructs:
 *   ge_Shape <name> Position|Portal|Range|Entity
 *   ge_Vector ...
 *
 * Stage 16 records declaration tails losslessly but does not assign coordinate
 * semantics to them.
 */
class ShapeScriptParser {
public:
	bool parse(const Common::String &decodedShapeScript,
	           ShapeScriptDocument &out,
	           Common::String &errorMessage) const;
};

ShapeDefinitionKind shapeDefinitionKind(const Common::String &type);
const char *shapeDefinitionKindName(ShapeDefinitionKind kind);

} // End of namespace ZeroComico

#endif
