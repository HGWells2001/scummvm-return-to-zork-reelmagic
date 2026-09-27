/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE6_SCRIPT_OPCODES_H
#define ZEROCOMICO_STAGE6_SCRIPT_OPCODES_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

class ScriptBridge;

enum Stage6OpcodeResult {
	kStage6OpcodeUnhandled,
	kStage6OpcodeDone,
	kStage6OpcodeYield,
	kStage6OpcodeBadArguments
};

/**
 * Adapter for the exact Lucifer command names recovered from Zero Comico.exe.
 *
 * This layer is intentionally parser-agnostic: Stage 3's VM can pass its
 * already-tokenized opcode + arguments here.
 */
Stage6OpcodeResult executeStage6Opcode(const Common::String &opcode,
                                       const Common::Array<Common::String> &args,
                                       ScriptBridge &bridge);

/**
 * Evaluate Stage 6 condition opcodes.
 *
 * Returns true when the opcode was recognized. The evaluated condition is
 * returned through value.
 */
bool evaluateStage6Condition(const Common::String &opcode,
                             const Common::Array<Common::String> &args,
                             ScriptBridge &bridge,
                             bool &value);

} // End of namespace ZeroComico

#endif
