/* Zero Comico Stage 6 focused self-test.
 *
 * Build this inside a ScummVM source tree after copying zerocomico-stage6/
 * to the source root, or adapt include paths when moving the files into
 * engines/zerocomico/.
 */

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage6/mainplace.h"
#include "zerocomico-stage6/mainplace_transition.h"
#include "zerocomico-stage6/menu_input.h"
#include "zerocomico-stage6/scene_picker.h"
#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage6/script_opcodes.h"
#include "zerocomico-stage6/timeline_eval.h"

#include <cassert>
#include <cmath>

using namespace ZeroComico;

static void testPicker() {
	ScenePicker picker;
	picker.resize(4, 4);

	assert(picker.pick(1, 1) == -1);
	assert(picker.writePixel(1, 1, 10.0f, 3));
	assert(picker.pick(1, 1) == 3);

	assert(!picker.writePixel(1, 1, 20.0f, 4));
	assert(picker.pick(1, 1) == 3);

	assert(picker.writePixel(1, 1, 5.0f, 4));
	assert(picker.pick(1, 1) == 4);
}

static void testMenuInput() {
	ScenePicker picker;
	picker.resize(8, 8);
	picker.writePixel(2, 3, 1.0f, 0);

	Common::Array<Common::String> names;
	names.push_back("int_TEST");

	MenuInput input;
	input.setMeshNames(&names);

	Common::Event ev;
	ev.type = Common::EVENT_LBUTTONDOWN;
	ev.mouse = Common::Point(2, 3);
	input.handleEvent(ev, picker);
	assert(input.pressedObject().equalsIgnoreCase("int_TEST"));

	ev.type = Common::EVENT_LBUTTONUP;
	input.handleEvent(ev, picker);
	assert(input.isActivated("int_TEST"));

	Common::String activated = input.consumeActivatedObject();
	assert(activated.equalsIgnoreCase("int_TEST"));
	assert(!input.isActivated("int_TEST"));
}

static void testTcb() {
	Common::Array<TcbVec3Key> keys;

	TcbVec3Key a;
	a.frame = 0;
	a.tension = a.continuity = a.bias = 0.0f;
	a.value = Vec3f(0.0f, 0.0f, 0.0f);
	keys.push_back(a);

	TcbVec3Key b = a;
	b.frame = 10;
	b.value = Vec3f(10.0f, 0.0f, 0.0f);
	keys.push_back(b);

	const Vec3f mid = evaluateTcbVec3(keys, 5.0f, Vec3f());
	assert(std::fabs(mid.x - 5.0f) < 0.001f);
	assert(std::fabs(mid.y) < 0.001f);
	assert(std::fabs(mid.z) < 0.001f);
}

static void testMainPlace() {
	const Common::String script(
		"ge_MainPlace Mp1\n"
		"{\n"
		"StartPlace: Room1_1\n"
		"Room Room1_1 { }\n"
		"Room Room1_2 { }\n"
		"}\n");

	MainPlaceDescriptor d;
	assert(parseMainPlaceDescriptor(script, d));
	assert(d.name == "Mp1");
	assert(d.startPlace == "Room1_1");
	assert(d.rooms.size() == 2);
	assert(d.rooms[0] == "Room1_1");
	assert(d.rooms[1] == "Room1_2");
}

static void testSceneRuntime() {
	SceneRuntime runtime;
	const int32 id = runtime.registry().addObject("int_MAIN", kSceneObjectMesh);
	assert(id == 0);
	assert(runtime.registry().object(id)->visible());

	runtime.setObjectVisible("int_MAIN", false);
	assert(!runtime.registry().object(id)->visible());

	runtime.setObjectVisible("int_MAIN", true);
	assert(runtime.registry().object(id)->visible());

	runtime.requestMainPlace("mp1");
	assert(runtime.hasPendingMainPlace());
	assert(runtime.pendingMainPlace() == "Mp1");
	runtime.clearPendingMainPlace();
	assert(!runtime.hasPendingMainPlace());
}

static void testCutOpcodes() {
	SceneRuntime runtime;

	Common::Array<AnimationClip> clips;
	AnimationClip click;
	click.name = "CLICK";
	click.framesPerSecond = 10.0f;
	click.firstFrame = 0;
	click.lastFrame = 1;
	clips.push_back(click);
	runtime.setAnimationClips(&clips);

	ScriptBridge bridge;
	bridge.setRuntimeHost(&runtime);

	Common::Array<Common::String> args;
	args.push_back("CLICK");

	assert(executeStage6Opcode("play_cut", args, bridge) == kStage6OpcodeDone);
	assert(runtime.isAnimationPlaying("CLICK"));
	assert(executeStage6Opcode("wait_cut", args, bridge) == kStage6OpcodeYield);

	runtime.update(250);
	assert(!runtime.isAnimationPlaying("CLICK"));
	assert(executeStage6Opcode("wait_cut", args, bridge) == kStage6OpcodeDone);
}

class FakeMainPlaceHost : public MainPlaceTransitionHost {
public:
	FakeMainPlaceHost() : activated(false) {}

	bool readDecodedText(const Common::Path &path, Common::String &text) override {
		lastPath = path;
		text =
			"ge_MainPlace Mp1\n"
			"{\n"
			"StartPlace: Room1_1\n"
			"Room Room1_1 { }\n"
			"Room Room1_2 { }\n"
			"}\n";
		return true;
	}

	bool activateMainPlace(const Common::String &directoryName,
	                       const MainPlaceDescriptor &descriptor,
	                       Common::String &errorMessage) override {
		(void)errorMessage;
		activated = true;
		activeDirectory = directoryName;
		activeDescriptor = descriptor;
		return true;
	}

	Common::Path lastPath;
	bool activated;
	Common::String activeDirectory;
	MainPlaceDescriptor activeDescriptor;
};

static void testChangeMainPlaceOpcode() {
	SceneRuntime runtime;
	ScriptBridge bridge;
	bridge.setRuntimeHost(&runtime);

	Common::Array<Common::String> args;
	args.push_back("Mp1");

	assert(executeStage6Opcode("ChangeMainplace", args, bridge) == kStage6OpcodeDone);
	assert(runtime.pendingMainPlace() == "Mp1");

	FakeMainPlaceHost host;
	MainPlaceTransitionController transition;
	assert(transition.process(runtime, host) == kMainPlaceTransitionDone);
	assert(host.activated);
	assert(host.activeDirectory == "Mp1");
	assert(host.activeDescriptor.name == "Mp1");
	assert(host.activeDescriptor.startPlace == "Room1_1");
	assert(!runtime.hasPendingMainPlace());
}

int main() {
	testPicker();
	testMenuInput();
	testTcb();
	testMainPlace();
	testSceneRuntime();
	testCutOpcodes();
	testChangeMainPlaceOpcode();
	return 0;
}
