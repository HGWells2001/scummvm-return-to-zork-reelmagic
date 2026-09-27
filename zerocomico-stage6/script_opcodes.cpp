/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage6/script_opcodes.h"
#include "zerocomico-stage6/script_bridge.h"

namespace ZeroComico {

static bool isOpcode(const Common::String &opcode, const char *name) {
	return opcode.equalsIgnoreCase(name);
}

Stage6OpcodeResult executeStage6Opcode(const Common::String &opcode,
                                       const Common::Array<Common::String> &args,
                                       ScriptBridge &bridge) {
	if (isOpcode(opcode, "E3D_hide")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		bridge.e3dHide(args[0]);
		return kStage6OpcodeDone;
	}

	if (isOpcode(opcode, "E3D_unhide")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		bridge.e3dUnhide(args[0]);
		return kStage6OpcodeDone;
	}

	if (isOpcode(opcode, "setfocus")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		bridge.setFocus(args[0]);
		return kStage6OpcodeDone;
	}

	if (isOpcode(opcode, "play_cut")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		return bridge.playCut(args[0]) ? kStage6OpcodeDone : kStage6OpcodeBadArguments;
	}

	if (isOpcode(opcode, "play_open_cut")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		return bridge.playOpenCut(args[0]) ? kStage6OpcodeDone : kStage6OpcodeBadArguments;
	}

	if (isOpcode(opcode, "loop_cut")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		return bridge.loopCut(args[0]) ? kStage6OpcodeDone : kStage6OpcodeBadArguments;
	}

	if (isOpcode(opcode, "wait_cut")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		return bridge.waitCut(args[0]) ? kStage6OpcodeYield : kStage6OpcodeDone;
	}

	if (isOpcode(opcode, "stop_cut")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		bridge.stopCut(args[0]);
		return kStage6OpcodeDone;
	}

	if (isOpcode(opcode, "ChangeMainplace")) {
		if (args.size() != 1)
			return kStage6OpcodeBadArguments;
		bridge.changeMainPlace(args[0]);
		return kStage6OpcodeDone;
	}

	return kStage6OpcodeUnhandled;
}

bool evaluateStage6Condition(const Common::String &opcode,
                             const Common::Array<Common::String> &args,
                             ScriptBridge &bridge,
                             bool &value) {
	if (isOpcode(opcode, "ifobjselected")) {
		if (args.size() == 2) {
			value = bridge.ifObjSelected(args[0], args[1]);
			return true;
		}

		// Compatibility with the Stage 3 WIP parser if it already normalized
		// away the owner/context token.
		if (args.size() == 1) {
			value = bridge.ifObjSelected(args[0]);
			return true;
		}

		value = false;
		return true;
	}

	if (isOpcode(opcode, "if_cutisfinished")) {
		if (args.size() != 1) {
			value = false;
			return true;
		}
		value = !bridge.waitCut(args[0]);
		return true;
	}

	return false;
}

} // End of namespace ZeroComico
