/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage8/object_handler_vm.h"

#include "common/tokenizer.h"

#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage6/script_opcodes.h"

namespace ZeroComico {

namespace {

static Common::String cleanToken(Common::String token) {
	token.trim();
	while (!token.empty() &&
	       (token.lastChar() == ':' || token.lastChar() == ';' ||
	        token.lastChar() == ',' || token.lastChar() == '{' ||
	        token.lastChar() == '}'))
		token.deleteLastChar();
	while (!token.empty() && (token.firstChar() == '"' || token.firstChar() == '\''))
		token.deleteChar(0);
	while (!token.empty() && (token.lastChar() == '"' || token.lastChar() == '\''))
		token.deleteLastChar();
	return token;
}

static void tokenize(const Common::String &line,
                     Common::String &opcode,
                     Common::Array<Common::String> &args) {
	opcode.clear();
	args.clear();

	Common::StringTokenizer tokens(line);
	if (tokens.empty())
		return;

	opcode = cleanToken(tokens.nextToken());
	while (!tokens.empty())
		args.push_back(cleanToken(tokens.nextToken()));
}

static bool isStructuralNoop(const Common::String &opcode) {
	return opcode.empty() ||
	       opcode == "{" || opcode == "}" ||
	       opcode.equalsIgnoreCase("begin_thread") ||
	       opcode.equalsIgnoreCase("end_thread") ||
	       opcode.equalsIgnoreCase("end") ||
	       opcode.equalsIgnoreCase("end.");
}

} // namespace

ObjectHandlerVM::ObjectHandlerVM() :
	_body(nullptr),
	_variables(nullptr),
	_bridge(nullptr),
	_host(nullptr),
	_pc(0) {
}

void ObjectHandlerVM::clear() {
	_body = nullptr;
	_variables = nullptr;
	_bridge = nullptr;
	_host = nullptr;
	_pc = 0;
	_conditions.clear();
	_blockedOpcode.clear();
}

void ObjectHandlerVM::begin(const ObjectHandlerBody *body,
                            GameplayVariables *variables,
                            ScriptBridge *bridge,
                            GameplayHandlerHost *host) {
	clear();
	_body = body;
	_variables = variables;
	_bridge = bridge;
	_host = host;
}

bool ObjectHandlerVM::executionEnabled() const {
	if (_conditions.empty())
		return true;

	const ConditionalFrame &frame = _conditions.back();
	const bool branch = frame.inElse ? !frame.condition : frame.condition;
	return frame.parentActive && branch;
}

void ObjectHandlerVM::finish() {
	_body = nullptr;
	_conditions.clear();
}

ObjectHandlerVmResult ObjectHandlerVM::executeLine(const Common::String &line,
                                                   bool &advance) {
	advance = true;

	Common::String opcode;
	Common::Array<Common::String> args;
	tokenize(line, opcode, args);

	if (isStructuralNoop(opcode))
		return kObjectHandlerVmRunning;

	if (opcode.equalsIgnoreCase("if_e")) {
		if (args.size() != 2 || !_variables)
			return kObjectHandlerVmBadArguments;

		ConditionalFrame frame;
		frame.parentActive = executionEnabled();
		frame.condition = frame.parentActive && _variables->equals(args[0], args[1]);
		_conditions.push_back(frame);
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("else")) {
		if (_conditions.empty())
			return kObjectHandlerVmBadArguments;
		_conditions.back().inElse = !_conditions.back().inElse;
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("endif")) {
		if (_conditions.empty())
			return kObjectHandlerVmBadArguments;
		_conditions.pop_back();
		return kObjectHandlerVmRunning;
	}

	if (!executionEnabled())
		return kObjectHandlerVmRunning;

	if (opcode.equalsIgnoreCase("mov")) {
		if (args.size() != 2 || !_variables)
			return kObjectHandlerVmBadArguments;
		_variables->set(args[0], args[1]);
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("hide")) {
		if (args.size() != 1 || !_bridge)
			return kObjectHandlerVmBadArguments;
		_bridge->e3dHide(args[0]);
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("unhide")) {
		if (args.size() != 1 || !_bridge)
			return kObjectHandlerVmBadArguments;
		_bridge->e3dUnhide(args[0]);
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("start_dialog")) {
		if (args.size() != 2 || !_host)
			return kObjectHandlerVmBadArguments;
		if (!_host->startDialog(args[0], args[1])) {
			_blockedOpcode = opcode;
			return kObjectHandlerVmBlockedOpcode;
		}
		return kObjectHandlerVmRunning;
	}

	if (opcode.equalsIgnoreCase("wait_last_dialog")) {
		if (!_host || !args.empty())
			return kObjectHandlerVmBadArguments;
		if (_host->isDialogPlaying()) {
			advance = false;
			return kObjectHandlerVmYield;
		}
		return kObjectHandlerVmRunning;
	}

	if (_bridge) {
		const Stage6OpcodeResult stage6 = executeStage6Opcode(opcode, args, *_bridge);
		if (stage6 == kStage6OpcodeDone)
			return kObjectHandlerVmRunning;
		if (stage6 == kStage6OpcodeYield) {
			advance = false;
			return kObjectHandlerVmYield;
		}
		if (stage6 == kStage6OpcodeBadArguments)
			return kObjectHandlerVmBadArguments;
	}

	_blockedOpcode = opcode;
	return kObjectHandlerVmBlockedOpcode;
}

ObjectHandlerVmResult ObjectHandlerVM::update(uint32 instructionBudget) {
	if (!_body)
		return kObjectHandlerVmIdle;

	for (uint32 count = 0; count < instructionBudget; ++count) {
		if (_pc >= _body->lines.size()) {
			finish();
			return kObjectHandlerVmDone;
		}

		bool advance = true;
		const ObjectHandlerVmResult result = executeLine(_body->lines[_pc], advance);
		if (result == kObjectHandlerVmBlockedOpcode ||
		    result == kObjectHandlerVmBadArguments)
			advance = false;

		if (advance)
			++_pc;

		if (result == kObjectHandlerVmYield ||
		    result == kObjectHandlerVmBlockedOpcode ||
		    result == kObjectHandlerVmBadArguments)
			return result;
	}

	return kObjectHandlerVmRunning;
}

} // End of namespace ZeroComico
