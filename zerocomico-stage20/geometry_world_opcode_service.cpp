/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage20/geometry_world_opcode_service.h"

namespace ZeroComico {

GeometryWorldOpcodeService::GeometryWorldOpcodeService() :
	_shapes(nullptr),
	_geometry(nullptr),
	_host(nullptr) {
}

void GeometryWorldOpcodeService::bind(
		const ShapeScriptDocument *shapes,
		const ShapeGeometryDocument *geometry,
		GeometryWorldRuntimeHost *host) {
	_shapes = shapes;
	_geometry = geometry;
	_host = host;
	_lastError.clear();
}

void GeometryWorldOpcodeService::clear() {
	_shapes = nullptr;
	_geometry = nullptr;
	_host = nullptr;
	_lastError.clear();
}

ObjectHandlerExternalOpcodeResult
GeometryWorldOpcodeService::executeObjectHandlerOpcode(
		const Common::String &opcode,
		const Common::Array<Common::String> &args) {
	_lastError.clear();

	if (!opcode.equalsIgnoreCase("SetCharPos_Vector"))
		return kObjectHandlerExternalUnhandled;
	if (args.size() != 2)
		return kObjectHandlerExternalBadArguments;
	if (!_shapes || !_geometry || !_host)
		return kObjectHandlerExternalUnhandled;

	const ShapeDefinition *definition = _shapes->shape(args[1]);
	if (!definition) {
		_lastError = Common::String::format(
			"Position helper '%s' is not declared in Shape.shp",
			args[1].c_str());
		return kObjectHandlerExternalUnhandled;
	}
	if (definition->kind != kShapeDefinitionPosition) {
		_lastError = Common::String::format(
			"Shape '%s' is %s, not Position",
			definition->name.c_str(),
			shapeDefinitionKindName(definition->kind));
		return kObjectHandlerExternalUnhandled;
	}

	const ShapeGeometryRecord *geometry = _geometry->shape(args[1]);
	if (!geometry) {
		_lastError = Common::String::format(
			"Position helper '%s' has no Stage 19 geometry record",
			args[1].c_str());
		return kObjectHandlerExternalUnhandled;
	}
	if (geometry->kind != kShapeDefinitionPosition) {
		_lastError = Common::String::format(
			"Geometry for '%s' is %s, not Position",
			args[1].c_str(),
			shapeDefinitionKindName(geometry->kind));
		return kObjectHandlerExternalUnhandled;
	}

	if (!_host->placeCharacterAtPositionGeometry(
			args[0], *definition, *geometry)) {
		_lastError = Common::String::format(
			"World host could not place character '%s' at Position '%s'",
			args[0].c_str(), args[1].c_str());
		return kObjectHandlerExternalUnhandled;
	}

	return kObjectHandlerExternalDone;
}

} // End of namespace ZeroComico
