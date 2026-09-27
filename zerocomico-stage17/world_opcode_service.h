/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE17_WORLD_OPCODE_SERVICE_H
#define ZEROCOMICO_STAGE17_WORLD_OPCODE_SERVICE_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage8/object_handler_vm.h"
#include "zerocomico-stage16/shape_document.h"

namespace ZeroComico {

class WorldOpcodeRuntimeHost {
public:
	virtual ~WorldOpcodeRuntimeHost() {}

	virtual bool placeCharacterAtPositionShape(
		const Common::String &characterName,
		const ShapeDefinition &positionShape) = 0;
};

/**
 * Executes world opcodes whose retail argument form is proven.
 *
 * Current retail-grounded command:
 *   SetCharPos_Vector <character> <Position-shape>
 *
 * Every other opcode remains unhandled and therefore blocks visibly in the
 * Object handler VM.
 */
class WorldOpcodeService : public ObjectHandlerExternalOpcodeHost {
public:
	WorldOpcodeService();

	void bind(const ShapeScriptDocument *shapes,
	          WorldOpcodeRuntimeHost *host);
	void clear();

	ObjectHandlerExternalOpcodeResult executeObjectHandlerOpcode(
		const Common::String &opcode,
		const Common::Array<Common::String> &args) override;

	const Common::String &lastError() const { return _lastError; }

private:
	const ShapeScriptDocument *_shapes;
	WorldOpcodeRuntimeHost *_host;
	Common::String _lastError;
};

} // End of namespace ZeroComico

#endif
