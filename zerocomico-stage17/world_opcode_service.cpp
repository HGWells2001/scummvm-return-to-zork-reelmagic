/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage17/world_opcode_service.h"

namespace ZeroComico {

WorldOpcodeService::WorldOpcodeService() :
	_shapes(nullptr),
	_host(nullptr) {
}

void WorldOpcodeService::bind(const ShapeScriptDocument *shapes,
                              WorldOpcodeRuntimeHost *host) {
	_shapes = shapes;
	_host = host;
	_lastError.clear();
}

void WorldOpcodeService::clear() {
	_shapes = nullptr;
	_host = nullptr;
	_lastError.clear();
}

ObjectHandlerExternalOpcodeResult WorldOpcodeService::executeObjectHandlerOpcode(
		const Common::String &opcode,
		const Common::Array<Common::String> &args) {
	_lastError.clear();

	if (!opcode.equalsIgnoreCase("SetCharPos_Vector"))
		return kObjectHandlerExternalUnhandled;

	if (args.size() != 2)
		return kObjectHandlerExternalBadArguments;
	if (!_shapes || !_host)
		return kObjectHandlerExternalUnhandled;

	const ShapeDefinition *shape = _shapes->shape(args[1]);
	if (!shape) {
		_lastError = Common::String::format(
			"Position helper '%s' is not declared in Shape.shp",
			args[1].c_str());
		return kObjectHandlerExternalUnhandled;
	}
	if (shape->kind != kShapeDefinitionPosition) {
		_lastError = Common::String::format(
			"Shape '%s' is %s, not Position",
			shape->name.c_str(), shapeDefinitionKindName(shape->kind));
		return kObjectHandlerExternalUnhandled;
	}

	if (!_host->placeCharacterAtPositionShape(args[0], *shape)) {
		_lastError = Common::String::format(
			"World host could not place character '%s' at '%s'",
			args[0].c_str(), shape->name.c_str());
		return kObjectHandlerExternalUnhandled;
	}

	return kObjectHandlerExternalDone;
}

} // End of namespace ZeroComico
