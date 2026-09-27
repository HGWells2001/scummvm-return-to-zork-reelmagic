/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE8_OBJECT_HANDLER_VM_H
#define ZEROCOMICO_STAGE8_OBJECT_HANDLER_VM_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage8/gameplay_variables.h"
#include "zerocomico-stage8/object_handlers.h"

namespace ZeroComico {

class ScriptBridge;

class GameplayHandlerHost {
public:
	virtual ~GameplayHandlerHost() {}

	virtual bool startDialog(const Common::String &speaker,
	                         const Common::String &dialogName) = 0;
	virtual bool isDialogPlaying() const = 0;
};

enum ObjectHandlerVmResult {
	kObjectHandlerVmIdle,
	kObjectHandlerVmRunning,
	kObjectHandlerVmYield,
	kObjectHandlerVmDone,
	kObjectHandlerVmBlockedOpcode,
	kObjectHandlerVmBadArguments
};

class ObjectHandlerVM {
public:
	ObjectHandlerVM();

	void clear();
	void begin(const ObjectHandlerBody *body,
	           GameplayVariables *variables,
	           ScriptBridge *bridge,
	           GameplayHandlerHost *host);

	ObjectHandlerVmResult update(uint32 instructionBudget = 128);

	bool active() const { return _body != nullptr; }
	const Common::String &blockedOpcode() const { return _blockedOpcode; }
	uint32 programCounter() const { return _pc; }

private:
	struct ConditionalFrame {
		bool parentActive;
		bool condition;
		bool inElse;

		ConditionalFrame() : parentActive(true), condition(false), inElse(false) {}
	};

	bool executionEnabled() const;
	ObjectHandlerVmResult executeLine(const Common::String &line, bool &advance);
	void finish();

	const ObjectHandlerBody *_body;
	GameplayVariables *_variables;
	ScriptBridge *_bridge;
	GameplayHandlerHost *_host;
	uint32 _pc;
	Common::Array<ConditionalFrame> _conditions;
	Common::String _blockedOpcode;
};

} // End of namespace ZeroComico

#endif
