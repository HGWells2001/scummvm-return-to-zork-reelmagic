/* Zero Comico Stage 24 gameplay room-state transition self-test. */

#include <cassert>

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage19/shape_geometry.h"
#include "zerocomico-stage21/room_topology.h"
#include "zerocomico-stage24/gameplay_room_transition.h"

using namespace ZeroComico;

static const char *kBspOne =
	"scene\nroom\npoly\n4\n"
	"0 0\n10 0\n10 10\n0 10\n0\nscene_end\n"
	"bsp\nbsp_points\n4\n"
	"0 0\n10 0\n10 10\n0 10\n"
	"bsp_edges\n4\n"
	"0 1 0 -1\n1 2 0 -1\n2 3 0 -1\n3 0 0 -1\n"
	"bsp_polygons\n1\n0\n4\n0\n1\n2\n3\n"
	"bsp_tree\n0 0 0\n-1\n-1\nbsp_end\n"
	"pathfinding\ngraph\n1\n"
	"5 5\n-1\n"
	"support\n0\npathfinding_end\n";

static const char *kBspTwo =
	"scene\nroom\npoly\n4\n"
	"0 0\n10 0\n10 10\n0 10\n0\nscene_end\n"
	"bsp\nbsp_points\n4\n"
	"0 0\n10 0\n10 10\n0 10\n"
	"bsp_edges\n4\n"
	"0 1 0 -1\n1 2 0 -1\n2 3 0 -1\n3 0 0 -1\n"
	"bsp_polygons\n1\n0\n4\n0\n1\n2\n3\n"
	"bsp_tree\n0 0 0\n-1\n-1\nbsp_end\n"
	"pathfinding\ngraph\n2\n"
	"1 1\n1 8 -1\n"
	"9 1\n0 8 -1\n"
	"support\n0\npathfinding_end\n";

static const char *kRooms =
	"ge_MainPlace Mp1\n"
	"{\n"
	"StartPlace: Room1_1\n"
	"Room Room1_1\n"
	"{\n"
	" Prefix: r11_\n"
	" backgrd: \"r11_Back\"\n"
	" map: r11_Map00\n"
	" cameramap: r11_MapCam00\n"
	" camera: r11_Camera01\n"
	" cameraspot: r11_Spot01\n"
	" portal: p11_to_12 Room1_2 entry \"vis one\" enabled\n"
	"}\n"
	"Room Room1_2\n"
	"{\n"
	" Prefix: r12_\n"
	" backgrd: \"r12_Back\"\n"
	" map: r12_Map00\n"
	" cameramap: r12_MapCam00\n"
	" camera: r12_Camera03\n"
	" cameraspot: r12_Spot03\n"
	" portal: p12_to_11 Room1_1 entry \"vis two\" enabled\n"
	"}\n"
	"}\n";

static const char *kShapes =
	"ge_Shape p11_to_12 Portal\n"
	"A 1 0 0\n"
	"B 2 0 0\n"
	"ge_Shape p12_to_11 Portal\n"
	"A 2 0 0\n"
	"B 1 0 0\n";

class FakeResources : public GameplayResourceHost {
public:
	FakeResources() :
		failR11(false),
		failR12(false) {}

	bool readDecodedText(const Common::Path &, Common::String &) override {
		return false;
	}

	bool readPlainText(const Common::Path &path, Common::String &text) override {
		const Common::String p = path.toString();

		if (p == "Mp1/gameplay/r11_Map00.bsp" ||
		    p == "Mp1/gameplay/r11_MapCam00.bsp") {
			if (failR11)
				return false;
			text = kBspOne;
			return true;
		}

		if (p == "Mp1/gameplay/r12_Map00.bsp" ||
		    p == "Mp1/gameplay/r12_MapCam00.bsp") {
			if (failR12)
				return false;
			text = kBspTwo;
			return true;
		}

		return false;
	}

	bool failR11;
	bool failR12;
};

class FakeScene : public GameplayRoomSceneHost {
public:
	FakeScene() : accept(true), calls(0) {}

	bool activateGameplayRoomScene(
			const ResolvedRoomTransition &transition,
			const PreparedRoomNavigation &navigation,
			Common::String &errorMessage) override {
		++calls;
		lastTarget = transition.targetRoom;
		lastActorNodes = navigation.actorNavigation.graph().size();
		lastCameraNodes = navigation.cameraNavigationAvailable
			? navigation.cameraNavigation.graph().size() : 0;

		if (!accept) {
			errorMessage = "renderer refused target room";
			return false;
		}
		errorMessage.clear();
		return true;
	}

	bool accept;
	int calls;
	Common::String lastTarget;
	uint32 lastActorNodes;
	uint32 lastCameraNodes;
};

static void buildDocuments(RoomTopologyDocument &topology,
                           ShapeScriptDocument &shapes,
                           ShapeGeometryDocument &geometry) {
	Common::String error;
	RoomTopologyParser roomParser;
	ShapeScriptParser shapeParser;
	ShapeGeometryParser geometryParser;

	assert(roomParser.parse(kRooms, topology, error));
	assert(shapeParser.parse(kShapes, shapes, error));
	assert(geometryParser.parse(kShapes, geometry, error));
}

static GameplayMainPlaceState makeState() {
	GameplayMainPlaceState state;
	state.mainPlace = "Mp1";

	Common::String error;
	assert(state.rooms.parse(kRooms, error));

	const GameplayRoomDescriptor *start =
		state.rooms.room(state.rooms.find("Room1_1"));
	assert(start);
	state.activeRoom = *start;

	assert(state.navigation.parse(kBspOne, error));
	assert(state.navigation.graph().size() == 1);
	return state;
}

static void testSuccessfulPortalChangesGameplayState() {
	RoomTopologyDocument topology;
	ShapeScriptDocument shapes;
	ShapeGeometryDocument geometry;
	buildDocuments(topology, shapes, geometry);

	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	FakeScene scene;

	GameplayRoomTransitionController controller;
	Common::String error;
	assert(controller.prepare(&state, topology, shapes, &geometry,
	                          &resources, &scene, error));
	assert(error.empty());
	assert(controller.currentRoom() == "Room1_1");
	assert(controller.graph().transitions().size() == 2);

	assert(controller.activatePortal("p11_to_12") ==
	       kRoomTransitionDone);

	assert(controller.currentRoom() == "Room1_2");
	assert(state.activeRoom.name == "Room1_2");
	assert(state.activeRoom.camera == "r12_Camera03");
	assert(state.navigation.graph().size() == 2);

	assert(controller.currentNavigation().roomName == "Room1_2");
	assert(controller.currentNavigation().actorNavigation.graph().size() == 2);
	assert(controller.currentNavigation().cameraNavigationAvailable);
	assert(controller.currentNavigation().cameraNavigation.graph().size() == 2);

	assert(scene.calls == 1);
	assert(scene.lastTarget == "Room1_2");
	assert(scene.lastActorNodes == 2);
	assert(scene.lastCameraNodes == 2);
}

static void testRendererRejectionRollsEverythingBack() {
	RoomTopologyDocument topology;
	ShapeScriptDocument shapes;
	ShapeGeometryDocument geometry;
	buildDocuments(topology, shapes, geometry);

	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	FakeScene scene;

	GameplayRoomTransitionController controller;
	Common::String error;
	assert(controller.prepare(&state, topology, shapes, &geometry,
	                          &resources, &scene, error));
	assert(controller.activatePortal("p11_to_12") == kRoomTransitionDone);

	scene.accept = false;
	assert(controller.activatePortal("p12_to_11") ==
	       kRoomTransitionHostRejected);

	assert(controller.currentRoom() == "Room1_2");
	assert(state.activeRoom.name == "Room1_2");
	assert(state.navigation.graph().size() == 2);
	assert(controller.currentNavigation().roomName == "Room1_2");
	assert(controller.currentNavigation().actorNavigation.graph().size() == 2);
	assert(controller.lastError() == "renderer refused target room");
}

static void testNavigationLoadFailureRollsBackBeforeScene() {
	RoomTopologyDocument topology;
	ShapeScriptDocument shapes;
	ShapeGeometryDocument geometry;
	buildDocuments(topology, shapes, geometry);

	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	FakeScene scene;

	GameplayRoomTransitionController controller;
	Common::String error;
	assert(controller.prepare(&state, topology, shapes, &geometry,
	                          &resources, &scene, error));
	assert(controller.activatePortal("p11_to_12") == kRoomTransitionDone);
	assert(scene.calls == 1);

	resources.failR11 = true;
	assert(controller.activatePortal("p12_to_11") ==
	       kRoomTransitionHostRejected);

	assert(scene.calls == 1);
	assert(controller.currentRoom() == "Room1_2");
	assert(state.activeRoom.name == "Room1_2");
	assert(state.navigation.graph().size() == 2);
	assert(!controller.lastError().empty());
}

static void testSuccessfulReturnUsesOldRoomBsp() {
	RoomTopologyDocument topology;
	ShapeScriptDocument shapes;
	ShapeGeometryDocument geometry;
	buildDocuments(topology, shapes, geometry);

	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	FakeScene scene;

	GameplayRoomTransitionController controller;
	Common::String error;
	assert(controller.prepare(&state, topology, shapes, &geometry,
	                          &resources, &scene, error));

	assert(controller.activatePortal("p11_to_12") == kRoomTransitionDone);
	assert(controller.activatePortal("p12_to_11") == kRoomTransitionDone);

	assert(controller.currentRoom() == "Room1_1");
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 1);
	assert(controller.currentNavigation().roomName == "Room1_1");
	assert(controller.currentNavigation().actorNavigation.graph().size() == 1);
	assert(scene.calls == 2);
}

static void testUnknownPortalLeavesStateAlone() {
	RoomTopologyDocument topology;
	ShapeScriptDocument shapes;
	ShapeGeometryDocument geometry;
	buildDocuments(topology, shapes, geometry);

	GameplayMainPlaceState state = makeState();
	FakeResources resources;
	FakeScene scene;

	GameplayRoomTransitionController controller;
	Common::String error;
	assert(controller.prepare(&state, topology, shapes, &geometry,
	                          &resources, &scene, error));

	assert(controller.activatePortal("missing") == kRoomTransitionNotFound);
	assert(controller.currentRoom() == "Room1_1");
	assert(state.activeRoom.name == "Room1_1");
	assert(state.navigation.graph().size() == 1);
	assert(scene.calls == 0);
}

int main() {
	testSuccessfulPortalChangesGameplayState();
	testRendererRejectionRollsEverythingBack();
	testNavigationLoadFailureRollsBackBeforeScene();
	testSuccessfulReturnUsesOldRoomBsp();
	testUnknownPortalLeavesStateAlone();
	return 0;
}
