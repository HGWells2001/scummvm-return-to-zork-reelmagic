/* Zero Comico Stage 8 focused self-test.
 *
 * The CI syntax-checks this against current ScummVM headers. The assertions
 * are also suitable for an engine-side unit-test harness.
 */

#include <cassert>
#include <cmath>

#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage8/gameplay_runtime.h"
#include "zerocomico-stage8/gameplay_variables.h"
#include "zerocomico-stage8/object_handler_vm.h"
#include "zerocomico-stage8/object_handlers.h"
#include "zerocomico-stage8/shared_actor.h"

using namespace ZeroComico;

static const char *kTinyBsp =
	"scene\n"
	"room\n"
	"poly\n"
	"4\n"
	"0 0\n"
	"10 0\n"
	"10 10\n"
	"0 10\n"
	"0\n"
	"scene_end\n"
	"bsp\n"
	"bsp_points\n"
	"4\n"
	"0 0\n"
	"10 0\n"
	"10 10\n"
	"0 10\n"
	"bsp_edges\n"
	"4\n"
	"0 1 0 -1\n"
	"1 2 0 -1\n"
	"2 3 0 -1\n"
	"3 0 0 -1\n"
	"bsp_polygons\n"
	"1\n"
	"0\n"
	"4\n"
	"0\n"
	"1\n"
	"2\n"
	"3\n"
	"bsp_tree\n"
	"0 0 0\n"
	"-1\n"
	"-1\n"
	"bsp_end\n"
	"pathfinding\n"
	"graph\n"
	"2\n"
	"1 1\n"
	"1 8 -1\n"
	"9 1\n"
	"0 8 -1\n"
	"support\n"
	"0\n"
	"pathfinding_end\n";

static const char *kRoomScript =
	"ge_MainPlace Mp1\n"
	"{\n"
	"StartPlace: Room1_1\n"
	"Variable guarito_PacMan 0\n"
	"Variable parlato_PacMan 0\n"
	"Room Room1_1\n"
	"{\n"
	"Prefix: r11_\n"
	"camera: r11_Camera01\n"
	"}\n"
	"}\n";

static const char *kPuzzleScript =
	"Object pac_pacman\n"
	"{\n"
	"PICKABLE 0\n"
	"EXAMINABLE 1\n"
	"OPERATED 1\n"
	"ENABLED 1\n"
	"entity: pac_pacman\n"
	"range: 2\n"
	"oprange: 1.5\n"
	"examine_text: \"Sembra affamato.\"\n"
	"operate:\n"
	"begin_thread\n"
	"if_e guarito_PacMan 0\n"
	"hide c121_pacman\n"
	"start_dialog giovanni Pacman_conpoz\n"
	"wait_last_dialog\n"
	"mov guarito_PacMan 1\n"
	"else\n"
	"start_dialog giovanni Pacman_ritparl\n"
	"endif\n"
	"end_thread\n"
	"}\n";

class FakeSceneHost : public ScriptRuntimeHost {
public:
	FakeSceneHost() : hidden(false) {}

	void setObjectVisible(const Common::String &objectName, bool visible) override {
		if (objectName.equalsIgnoreCase("c121_pacman"))
			hidden = !visible;
	}
	bool playAnimation(const Common::String &, bool) override { return true; }
	bool isAnimationPlaying() const override { return false; }
	bool isAnimationPlaying(const Common::String &) const override { return false; }
	void stopAnimation(const Common::String &) override {}
	void setFocus(const Common::String &) override {}
	void requestMainPlace(const Common::String &) override {}

	bool hidden;
};

class FakeGameplayHost : public GameplayRuntimeHost, public GameplayHandlerHost {
public:
	FakeGameplayHost() :
		loaded(false),
		walking(false),
		dialogPlaying(false),
		dialogCount(0),
		lastPosition(0.0f, 0.0f) {
	}

	bool loadSharedActor(const SharedActorAssets &assets) override {
		loaded = assets.name == "Giovanni" &&
		         assets.model.toString() == "Mpx/bodies/Giovanni/Giovanni.p3d";
		return loaded;
	}

	bool resolveEntityFloorPosition(const Common::String &entityName,
	                                NavVec2 &position) const override {
		if (!entityName.equalsIgnoreCase("pac_pacman"))
			return false;
		position = NavVec2(9.0f, 1.0f);
		return true;
	}

	void setActorFloorPosition(const Common::String &actorName,
	                           const NavVec2 &position) override {
		assert(actorName == "Giovanni");
		lastPosition = position;
	}

	void setActorWalking(const Common::String &actorName, bool value) override {
		assert(actorName == "Giovanni");
		walking = value;
	}

	bool startDialog(const Common::String &speaker,
	                 const Common::String &dialogName) override {
		assert(speaker.equalsIgnoreCase("giovanni"));
		assert(!dialogName.empty());
		++dialogCount;
		dialogPlaying = true;
		return true;
	}

	bool isDialogPlaying() const override {
		return dialogPlaying;
	}

	void showExamineText(const Common::String &text) override {
		lastExamineText = text;
	}

	bool loaded;
	bool walking;
	bool dialogPlaying;
	int dialogCount;
	NavVec2 lastPosition;
	Common::String lastExamineText;
};

static void testSharedActorPaths() {
	const SharedActorAssets actor = makeSharedActorAssets("Giovanni");
	assert(actor.valid());
	assert(actor.model.toString() == "Mpx/bodies/Giovanni/Giovanni.p3d");
	assert(actor.animation.toString() == "Mpx/bodies/Giovanni/Giovanni.anj");
	assert(actor.sequence.toString() == "Mpx/bodies/Giovanni/Giovanni.seq");
	assert(actor.material.toString() == "Mpx/bodies/Giovanni/Giovanni.mat");
}

static void testVariablesAndHandlers() {
	GameplayVariables variables;
	variables.parseDeclarations(kRoomScript);
	assert(variables.size() == 2);
	assert(variables.equals("guarito_PacMan", "0"));

	ObjectHandlerRegistry handlers;
	handlers.parse(kPuzzleScript);
	const ObjectHandlerBody *body = handlers.find("pac_pacman", kGameplayOperate);
	assert(body);
	assert(!body->lines.empty());
}

static void testHandlerVm() {
	GameplayVariables variables;
	variables.parseDeclarations(kRoomScript);

	ObjectHandlerRegistry handlers;
	handlers.parse(kPuzzleScript);
	const ObjectHandlerBody *body = handlers.find("pac_pacman", kGameplayOperate);
	assert(body);

	FakeSceneHost scene;
	ScriptBridge bridge;
	bridge.setRuntimeHost(&scene);

	FakeGameplayHost gameplay;
	ObjectHandlerVM vm;
	vm.begin(body, &variables, &bridge, &gameplay);

	assert(vm.update() == kObjectHandlerVmYield);
	assert(scene.hidden);
	assert(gameplay.dialogCount == 1);
	assert(variables.equals("guarito_PacMan", "0"));

	gameplay.dialogPlaying = false;
	const ObjectHandlerVmResult result = vm.update();
	assert(result == kObjectHandlerVmDone || result == kObjectHandlerVmRunning);
	if (vm.active())
		assert(vm.update() == kObjectHandlerVmDone);
	assert(variables.equals("guarito_PacMan", "1"));
}

static void testGameplayRuntime() {
	GameplayMainPlaceState state;
	state.mainPlace = "Mp1";
	Common::String error;
	assert(state.navigation.parse(kTinyBsp, error));
	assert(state.objects.parse(kPuzzleScript, error));

	FakeSceneHost scene;
	ScriptBridge bridge;
	bridge.setRuntimeHost(&scene);

	FakeGameplayHost host;
	GameplayRuntime runtime;
	assert(runtime.prepare(&state, kRoomScript, kPuzzleScript, &host, &bridge, error));
	assert(host.loaded);

	runtime.setActorStartPosition(NavVec2(0.0f, 1.0f));
	runtime.setWalkSpeed(1000.0f);
	assert(runtime.interact("pac_pacman", kGameplayOperate) ==
	       kGameplayInteractionWalking);

	runtime.update(100);
	assert(!runtime.walking());
	assert(runtime.handlerVM().active());
	assert(scene.hidden);
	assert(host.dialogPlaying);

	host.dialogPlaying = false;
	runtime.update(16);
	assert(runtime.variables().equals("guarito_PacMan", "1"));

	const float dx = runtime.actorPosition().x - 9.0f;
	const float dy = runtime.actorPosition().y - 1.0f;
	const float distance = std::sqrt(dx * dx + dy * dy);
	assert(distance <= 1.5f);
}

int main() {
	testSharedActorPaths();
	testVariablesAndHandlers();
	testHandlerVm();
	testGameplayRuntime();
	return 0;
}
