/* Zero Comico Stage 17 world opcode self-test. */

#include <cassert>

#include "zerocomico-stage8/object_handler_vm.h"
#include "zerocomico-stage17/world_opcode_service.h"

using namespace ZeroComico;

class FakeGameplayHandler : public GameplayHandlerHost {
public:
	bool startDialog(const Common::String &, const Common::String &) override {
		return true;
	}
	bool isDialogPlaying() const override {
		return false;
	}
};

class FakeWorldHost : public WorldOpcodeRuntimeHost {
public:
	FakeWorldHost() :
		placed(false),
		portalsEnabled(true) {}

	bool placeCharacterAtPositionShape(
			const Common::String &characterName,
			const ShapeDefinition &positionShape) override {
		if (!characterName.equalsIgnoreCase("Giovanni"))
			return false;
		if (!positionShape.name.equalsIgnoreCase("StartPos"))
			return false;
		placed = true;
		lastCharacter = characterName;
		lastShape = positionShape.name;
		return true;
	}

	void setPortalsEnabled(bool enabled) override {
		portalsEnabled = enabled;
	}

	bool placed;
	bool portalsEnabled;
	Common::String lastCharacter;
	Common::String lastShape;
};

static ShapeScriptDocument makeShapes() {
	const char *shapeText =
		"ge_Shape StartPos Position\n"
		"ge_Shape DoorPortal Portal\n";

	ShapeScriptDocument shapes;
	ShapeScriptParser parser;
	Common::String error;
	assert(parser.parse(shapeText, shapes, error));
	assert(error.empty());
	return shapes;
}

static void testServiceDirectly() {
	ShapeScriptDocument shapes = makeShapes();
	FakeWorldHost world;
	WorldOpcodeService service;
	service.bind(&shapes, &world);

	Common::Array<Common::String> args;
	args.push_back("Giovanni");
	args.push_back("StartPos");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalDone);
	assert(world.placed);
	assert(world.lastCharacter == "Giovanni");
	assert(world.lastShape == "StartPos");

	args.clear();
	assert(service.executeObjectHandlerOpcode("portals_off", args) ==
	       kObjectHandlerExternalDone);
	assert(!world.portalsEnabled);
	assert(service.executeObjectHandlerOpcode("portals_on", args) ==
	       kObjectHandlerExternalDone);
	assert(world.portalsEnabled);

	args.push_back("unexpected");
	assert(service.executeObjectHandlerOpcode("portals_on", args) ==
	       kObjectHandlerExternalBadArguments);

	args.clear();
	args.push_back("Giovanni");
	args.push_back("DoorPortal");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalUnhandled);
	assert(!service.lastError().empty());

	args.clear();
	assert(service.executeObjectHandlerOpcode("setplace", args) ==
	       kObjectHandlerExternalUnhandled);
}

static void testVmExternalHookAndBlocking() {
	ShapeScriptDocument shapes = makeShapes();
	FakeWorldHost world;
	WorldOpcodeService service;
	service.bind(&shapes, &world);

	ObjectHandlerBody body;
	body.objectName = "test";
	body.verb = kGameplayOperate;
	body.lines.push_back("SetCharPos_Vector Giovanni StartPos");
	body.lines.push_back("portals_off");
	body.lines.push_back("portals_on");
	body.lines.push_back("setplace Room1_2");

	GameplayVariables variables;
	FakeGameplayHandler gameplay;
	ObjectHandlerVM vm;
	vm.begin(&body, &variables, nullptr, &gameplay, &service);

	const ObjectHandlerVmResult result = vm.update();
	assert(result == kObjectHandlerVmBlockedOpcode);
	assert(vm.active());
	assert(vm.blockedOpcode().equalsIgnoreCase("setplace"));
	assert(vm.programCounter() == 3);
	assert(world.placed);
	assert(world.portalsEnabled);

	// A second update must stop at the same unsupported instruction.
	assert(vm.update() == kObjectHandlerVmBlockedOpcode);
	assert(vm.programCounter() == 3);
}

static void testBadArgumentsStopVm() {
	ShapeScriptDocument shapes = makeShapes();
	FakeWorldHost world;
	WorldOpcodeService service;
	service.bind(&shapes, &world);

	ObjectHandlerBody body;
	body.objectName = "bad";
	body.verb = kGameplayOperate;
	body.lines.push_back("SetCharPos_Vector Giovanni");

	GameplayVariables variables;
	FakeGameplayHandler gameplay;
	ObjectHandlerVM vm;
	vm.begin(&body, &variables, nullptr, &gameplay, &service);

	assert(vm.update() == kObjectHandlerVmBadArguments);
	assert(vm.programCounter() == 0);
}

int main() {
	testServiceDirectly();
	testVmExternalHookAndBlocking();
	testBadArgumentsStopVm();
	return 0;
}
