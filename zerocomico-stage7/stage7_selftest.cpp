/* Zero Comico Stage 7 focused self-test.
 *
 * Syntax-checked by GitHub Actions against current ScummVM headers.
 */

#include "zerocomico-stage7/bsp_navigation.h"
#include "zerocomico-stage7/gameplay_interaction.h"
#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage7/gameplay_object.h"
#include "zerocomico-stage7/gameplay_room.h"
#include "zerocomico-stage7/path_follower.h"

#include <cassert>
#include <cmath>

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
	"Room Room1_1\n"
	"{\n"
	"Prefix: r11_\n"
	"backgrd: \"r11_Camera01\"\n"
	"camera: r11_Camera01\n"
	"}\n"
	"Room Room1_2\n"
	"{\n"
	"Prefix: r12_\n"
	"camera: r12_Camera01\n"
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
	"examine\n"
	"begin_thread\n"
	"end_thread\n"
	"operate\n"
	"begin_thread\n"
	"end_thread\n"
	"}\n";

static void testBsp() {
	BspNavigation nav;
	Common::String error;
	assert(nav.parse(kTinyBsp, error));
	assert(nav.points().size() == 4);
	assert(nav.edges().size() == 4);
	assert(nav.cells().size() == 1);
	assert(nav.graph().size() == 2);

	Common::Array<NavVec2> path;
	assert(nav.findPath(NavVec2(0, 1), NavVec2(10, 1), path));
	assert(path.size() >= 4);
}

static void testRoomDescriptor() {
	GameplayRoomRegistry rooms;
	Common::String error;
	assert(rooms.parse(kRoomScript, error));
	assert(rooms.size() == 2);

	const GameplayRoomDescriptor *room = rooms.room(rooms.find("Room1_1"));
	assert(room);
	assert(room->prefix == "r11_");
	assert(room->camera == "r11_Camera01");

	const Common::Path navPath = gameplayNavigationPath("Mp1", *room);
	assert(navPath.toString() == "Mp1/gameplay/r11_Map00.bsp");
}

static void testObjects() {
	GameplayObjectRegistry objects;
	Common::String error;
	assert(objects.parse(kPuzzleScript, error));
	assert(objects.size() == 1);

	const int32 id = objects.resolvePickedEntity("pac_pacman");
	const GameplayObject *object = objects.object(id);
	assert(object);
	assert(object->name == "pac_pacman");
	assert(object->examinable);
	assert(object->operated);
	assert(!object->pickable);
	assert(object->hasExamineHandler);
	assert(object->hasOperateHandler);
	assert(std::fabs(object->operateRange - 1.5f) < 0.001f);
}

class FakeInteractionHost : public GameplayInteractionHost {
public:
	FakeInteractionHost() : inRange(false), walkStarted(false), executed(false) {}

	bool isWithinObjectRange(const Common::String &entityName, float range) const override {
		return inRange && entityName == "pac_pacman" && range > 0.0f;
	}

	bool beginWalkToObject(const Common::String &entityName, float range) override {
		walkStarted = entityName == "pac_pacman" && range > 0.0f;
		return walkStarted;
	}

	bool executeObjectHandler(const Common::String &objectName, GameplayVerb verb) override {
		executed = objectName == "pac_pacman" && verb == kGameplayOperate;
		return executed;
	}

	bool inRange;
	bool walkStarted;
	bool executed;
};

static void testInteraction() {
	GameplayObjectRegistry objects;
	Common::String error;
	assert(objects.parse(kPuzzleScript, error));

	GameplayInteractionController controller;
	controller.setRegistry(&objects);

	FakeInteractionHost host;
	assert(controller.request("pac_pacman", kGameplayOperate, host) ==
	       kGameplayInteractionWalking);
	assert(host.walkStarted);
	assert(controller.hasPendingInteraction());

	host.inRange = true;
	assert(controller.finishWalk(true, host) == kGameplayInteractionExecuted);
	assert(host.executed);
}

static void testPathFollower() {
	BspNavigation nav;
	Common::String error;
	assert(nav.parse(kTinyBsp, error));

	PathFollower follower;
	assert(follower.begin(nav, NavVec2(0, 1), NavVec2(10, 1), 20.0f));
	for (int i = 0; i < 20 && follower.active(); ++i)
		follower.update(100);
	assert(!follower.active());
	assert(std::fabs(follower.position().x - 10.0f) < 0.001f);
}

class FakeResourceHost : public GameplayResourceHost {
public:
	bool readDecodedText(const Common::Path &path, Common::String &text) override {
		if (path.toString() == "Mp1/gameplay/room.isc") {
			text = kRoomScript;
			return true;
		}
		if (path.toString() == "Mp1/gameplay/puzzle.isc") {
			text = kPuzzleScript;
			return true;
		}
		return false;
	}

	bool readPlainText(const Common::Path &path, Common::String &text) override {
		if (path.toString() == "Mp1/gameplay/r11_Map00.bsp") {
			text = kTinyBsp;
			return true;
		}
		return false;
	}
};

static void testMainPlaceGameplayLoader() {
	MainPlaceDescriptor descriptor;
	assert(parseMainPlaceDescriptor(kRoomScript, descriptor));
	assert(descriptor.startPlace == "Room1_1");

	FakeResourceHost host;
	GameplayMainPlaceState state;
	GameplayMainPlaceLoader loader;
	Common::String error;
	assert(loader.load("Mp1", descriptor, host, state, error));
	assert(state.mainPlace == "Mp1");
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 2);
	assert(state.objects.size() == 1);
}

int main() {
	testBsp();
	testRoomDescriptor();
	testObjects();
	testInteraction();
	testPathFollower();
	testMainPlaceGameplayLoader();
	return 0;
}
