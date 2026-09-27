/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE20_GEOMETRY_WORLD_OPCODE_SERVICE_H
#define ZEROCOMICO_STAGE20_GEOMETRY_WORLD_OPCODE_SERVICE_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage8/object_handler_vm.h"
#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage19/shape_geometry.h"

namespace ZeroComico {

class GeometryWorldRuntimeHost {
public:
	virtual ~GeometryWorldRuntimeHost() {}

	virtual bool placeCharacterAtPositionGeometry(
		const Common::String &characterName,
		const ShapeDefinition &positionShape,
		const ShapeGeometryRecord &geometry) = 0;

	virtual void setPortalsEnabled(bool enabled) = 0;
};

/**
 * Geometry-aware successor to the Stage 17 world opcode service.
 *
 * It validates symbolic Shape.shp type information and joins it with the
 * Stage 19 numeric A/B payload before calling the world host.
 */
class GeometryWorldOpcodeService : public ObjectHandlerExternalOpcodeHost {
public:
	GeometryWorldOpcodeService();

	void bind(const ShapeScriptDocument *shapes,
	          const ShapeGeometryDocument *geometry,
	          GeometryWorldRuntimeHost *host);

	void clear();

	ObjectHandlerExternalOpcodeResult executeObjectHandlerOpcode(
		const Common::String &opcode,
		const Common::Array<Common::String> &args) override;

	const Common::String &lastError() const { return _lastError; }

private:
	const ShapeScriptDocument *_shapes;
	const ShapeGeometryDocument *_geometry;
	GeometryWorldRuntimeHost *_host;
	Common::String _lastError;
};

} // End of namespace ZeroComico

#endif
