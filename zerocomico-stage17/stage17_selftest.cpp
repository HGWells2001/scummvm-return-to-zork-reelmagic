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
	FakeWorldHost() : placed(false) {}

	bool placeCharacterAtPositionShape(
			const Common::String &characterName,
			const ShapeDefinition &positionShape) override {
		if (!characterName.equalsIgnoreCase("Pacman"))
			return false;
		if (!positionShape.name.equalsIgnoreCase("r12_Start_Pacman"))
			return false;
		placed = true;
		lastCharacter = characterName;
		lastShape = positionShape.name;
		return true;
	}

	bool placed;
	Common::String lastCharacter;
	Common::String lastShape;
};

static ShapeScriptDocument makeShapes() {
	const char *shapeText =
		"ge_Shape r12_Start_Pacman Position\n"
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
	args.push_back("Pacman");
	args.push_back("r12_Start_Pacman");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalDone);
	assert(world.placed);
	assert(world.lastCharacter == "Pacman");
	assert(world.lastShape == "r12_Start_Pacman");

	args.clear();
	args.push_back("Pacman");
	args.push_back("DoorPortal");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalUnhandled);
	assert(!service.lastError().empty());

	args.clear();
	assert(service.executeObjectHandlerOpcode("portals_on", args) ==
	       kObjectHandlerExternalUnhandled);
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
	body.lines.push_back("SetCharPos_Vector Pacman r12_Start_Pacman");
	body.lines.push_back("setplace Room1_2");

	GameplayVariables variables;
	FakeGameplayHandler gameplay;
	ObjectHandlerVM vm;
	vm.begin(&body, &variables, nullptr, &gameplay, &service);

	const ObjectHandlerVmResult result = vm.update();
	assert(result == kObjectHandlerVmBlockedOpcode);
	assert(vm.active());
	assert(vm.blockedOpcode().equalsIgnoreCase("setplace"));
	assert(vm.programCounter() == 1);
	assert(world.placed);

	// Repeated updates must remain parked on the same unsupported opcode.
	assert(vm.update() == kObjectHandlerVmBlockedOpcode);
	assert(vm.programCounter() == 1);
}

static void testBadArgumentsStopVm() {
	ShapeScriptDocument shapes = makeShapes();
	FakeWorldHost world;
	WorldOpcodeService service;
	service.bind(&shapes, &world);

	ObjectHandlerBody body;
	body.objectName = "bad";
	body.verb = kGameplayOperate;
	body.lines.push_back("SetCharPos_Vector Pacman");

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
