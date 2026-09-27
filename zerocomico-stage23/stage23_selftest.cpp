/* Zero Comico Stage 23 transactional gameplay room self-test. */

#include <cassert>

#include "zerocomico-stage23/gameplay_room_transition_host.h"

using namespace ZeroComico;

static const char *kRooms =
	"ge_MainPlace Mp1\n"
	"{\n"
	"Room Room1_1 {\n"
	" Prefix: r11_\n"
	" map: r11_Map00\n"
	" camera: r11_Camera01\n"
	"}\n"
	"Room Room1_2 {\n"
	" Prefix: r12_\n"
	" backgrd: r12_Background\n"
	" map: r12_Map00\n"
	" cameramap: r12_MapCam00\n"
	" camera: r12_Camera03\n"
	"}\n"
	"}\n";

static const char *kTinyBspA =
	"scene\nroom\npoly\n4\n0 0\n10 0\n10 10\n0 10\n0\nscene_end\n"
	"bsp\nbsp_points\n4\n0 0\n10 0\n10 10\n0 10\n"
	"bsp_edges\n4\n0 1 0 -1\n1 2 0 -1\n2 3 0 -1\n3 0 0 -1\n"
	"bsp_polygons\n1\n0\n4\n0\n1\n2\n3\n"
	"bsp_tree\n0 0 0\n-1\n-1\nbsp_end\n"
	"pathfinding\ngraph\n2\n1 1\n1 8 -1\n9 1\n0 8 -1\n"
	"support\n0\npathfinding_end\n";

static const char *kTinyBspB =
	"scene\nroom\npoly\n4\n0 0\n20 0\n20 20\n0 20\n0\nscene_end\n"
	"bsp\nbsp_points\n4\n0 0\n20 0\n20 20\n0 20\n"
	"bsp_edges\n4\n0 1 0 -1\n1 2 0 -1\n2 3 0 -1\n3 0 0 -1\n"
	"bsp_polygons\n1\n0\n4\n0\n1\n2\n3\n"
	"bsp_tree\n0 0 0\n-1\n-1\nbsp_end\n"
	"pathfinding\ngraph\n3\n1 1\n1 9 -1\n10 1\n0 9 2 9 -1\n19 1\n1 9 -1\n"
	"support\n0\npathfinding_end\n";

class FakeResources : public GameplayResourceHost {
public:
	FakeResources() : failRoom2(false), malformedRoom2(false) {}

	bool readDecodedText(const Common::Path &, Common::String &) override {
		return false;
	}

	bool readPlainText(const Common::Path &path, Common::String &text) override {
		if (path.toString() == "Mp1/gameplay/r11_Map00.bsp") {
			text = kTinyBspA;
			return true;
		}
		if (path.toString() == "Mp1/gameplay/r12_Map00.bsp") {
			if (failRoom2)
				return false;
			text = malformedRoom2 ? "broken bsp" : kTinyBspB;
			return true;
		}
		return false;
	}

	bool failRoom2;
	bool malformedRoom2;
};

static GameplayMainPlaceState makeState() {
	GameplayMainPlaceState state;
	state.mainPlace = "Mp1";

	Common::String error;
	assert(state.rooms.parse(kRooms, error));
	const GameplayRoomDescriptor *room1 =
		state.rooms.room(state.rooms.find("Room1_1"));
	assert(room1);
	state.activeRoom = *room1;
	assert(state.navigation.parse(kTinyBspA, error));
	return state;
}

static ResolvedRoomTransition room12Transition() {
	ResolvedRoomTransition t;
	t.sourceRoom = "Room1_1";
	t.targetRoom = "Room1_2";
	t.portalShape = "p11_to_12";
	t.targetPrefix = "r12_";
	t.targetBackground = "r12_Background";
	t.targetMap = "r12_Map00";
	t.targetCameraMap = "r12_MapCam00";
	t.targetCamera = "r12_Camera03";
	return t;
}

static void testSuccessfulCommit() {
	GameplayMainPlaceState state = makeState();
	const uint32 oldObjects = state.objects.size();
	FakeResources resources;
	GameplayRoomTransitionHost host;
	host.bind(&state, &resources);

	Common::String error;
	assert(host.activateResolvedRoom(room12Transition(), error));
	assert(error.empty());
	assert(state.activeRoom.name == "Room1_2");
	assert(state.activeRoom.prefix == "r12_");
	assert(state.activeRoom.navigationMap == "r12_Map00");
	assert(state.activeRoom.cameraMap == "r12_MapCam00");
	assert(state.activeRoom.camera == "r12_Camera03");
	assert(state.navigation.graph().size() == 3);

	// Object registry is MainPlace-wide and must survive Room switches.
	assert(state.objects.size() == oldObjects);
}

static void testMissingBspRollsBack() {
	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	resources.failRoom2 = true;

	GameplayRoomTransitionHost host;
	host.bind(&state, &resources);
	Common::String error;
	assert(!host.activateResolvedRoom(room12Transition(), error));
	assert(!error.empty());
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 2);
}

static void testMalformedBspRollsBack() {
	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	resources.malformedRoom2 = true;

	GameplayRoomTransitionHost host;
	host.bind(&state, &resources);
	Common::String error;
	assert(!host.activateResolvedRoom(room12Transition(), error));
	assert(!error.empty());
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 2);
}

static void testResourceMismatchRejected() {
	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	GameplayRoomTransitionHost host;
	host.bind(&state, &resources);

	ResolvedRoomTransition bad = room12Transition();
	bad.targetCamera = "wrong_camera";

	Common::String error;
	assert(!host.activateResolvedRoom(bad, error));
	assert(!error.empty());
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 2);
}

static void testUnknownRoomRejected() {
	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	GameplayRoomTransitionHost host;
	host.bind(&state, &resources);

	ResolvedRoomTransition bad = room12Transition();
	bad.targetRoom = "Room1_99";

	Common::String error;
	assert(!host.activateResolvedRoom(bad, error));
	assert(!error.empty());
	assert(state.activeRoom.name == "Room1_1");
}

int main() {
	testSuccessfulCommit();
	testMissingBspRollsBack();
	testMalformedBspRollsBack();
	testResourceMismatchRejected();
	testUnknownRoomRejected();
	return 0;
}
