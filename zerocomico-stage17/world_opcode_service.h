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

	virtual void setPortalsEnabled(bool enabled) = 0;
};

/**
 * Executes only world opcodes whose argument shape is currently grounded.
 *
 * Supported:
 *   SetCharPos_Vector <character> <Position-shape>
 *   portals_on
 *   portals_off
 *
 * Other known world opcodes remain unhandled until retail usage proves
 * their arity/order.
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
