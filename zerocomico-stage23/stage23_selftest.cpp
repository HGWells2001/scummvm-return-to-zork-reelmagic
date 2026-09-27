/* Zero Comico Stage 23 Map/MapCam bundle self-test. */

#include <cassert>

#include "zerocomico-stage23/room_navigation_bundle.h"

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

class FakeResources : public GameplayResourceHost {
public:
	FakeResources() :
		actorMapReadable(true),
		cameraMapReadable(true),
		actorMapEmpty(false),
		cameraMapEmpty(false) {}

	bool readDecodedText(const Common::Path &, Common::String &) override {
		return false;
	}

	bool readPlainText(const Common::Path &path, Common::String &text) override {
		const Common::String p = path.toString();
		if (p == "Mp1/gameplay/r12_Map00.bsp") {
			if (!actorMapReadable)
				return false;
			text = actorMapEmpty ? Common::String() : Common::String(kTinyBsp);
			return true;
		}
		if (p == "Mp1/gameplay/r12_MapCam00.bsp") {
			if (!cameraMapReadable)
				return false;
			text = cameraMapEmpty ? Common::String() : Common::String(kTinyBsp);
			return true;
		}
		return false;
	}

	bool actorMapReadable;
	bool cameraMapReadable;
	bool actorMapEmpty;
	bool cameraMapEmpty;
};

static ResolvedRoomTransition makeTransition() {
	ResolvedRoomTransition t;
	t.sourceRoom = "Room1_1";
	t.targetRoom = "Room1_2";
	t.portalShape = "p11_to_12";
	t.targetMap = "r12_Map00";
	t.targetCameraMap = "r12_MapCam00";
	t.targetCamera = "r12_Camera03";
	t.targetCameraSpot = "r12_Spot03";
	return t;
}

class FakeActivation : public RoomNavigationActivationHost {
public:
	FakeActivation() : accept(true), calls(0) {}

	bool activatePreparedRoom(
			const ResolvedRoomTransition &transition,
			const PreparedRoomNavigation &navigation,
			Common::String &errorMessage) override {
		++calls;
		lastTargetRoom = transition.targetRoom;
		lastNavigation = navigation;

		if (!accept) {
			errorMessage = "scene activation failed";
			return false;
		}
		errorMessage.clear();
		return true;
	}

	bool accept;
	int calls;
	Common::String lastTargetRoom;
	PreparedRoomNavigation lastNavigation;
};

static void testPathConstruction() {
	assert(roomBspResourcePath("mp1", "r12_Map00").toString() ==
	       "Mp1/gameplay/r12_Map00.bsp");
	assert(roomBspResourcePath("MP1", "r12_Map00.bsp").toString() ==
	       "Mp1/gameplay/r12_Map00.bsp");
	assert(roomBspResourcePath(
		"Mp1", "Mp1/gameplay/custom.bsp").toString() ==
	       "Mp1/gameplay/custom.bsp");
	assert(roomBspResourcePath("Mp1", "").empty());
}

static void testTwoNavigationPlans() {
	FakeResources resources;
	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	Common::String error;

	assert(loader.load("Mp1", makeTransition(), resources,
	                   prepared, error));
	assert(error.empty());
	assert(prepared.roomName == "Room1_2");
	assert(prepared.actorNavigation.graph().size() == 2);
	assert(prepared.cameraMapDeclared);
	assert(!prepared.cameraMapEmpty);
	assert(prepared.cameraNavigationAvailable);
	assert(prepared.cameraNavigation.graph().size() == 2);
}

static void testRetailEmptyCameraMapAllowed() {
	FakeResources resources;
	resources.cameraMapEmpty = true;

	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	Common::String error;
	assert(loader.load("Mp1", makeTransition(), resources,
	                   prepared, error));
	assert(error.empty());
	assert(prepared.actorNavigation.graph().size() == 2);
	assert(prepared.cameraMapDeclared);
	assert(prepared.cameraMapEmpty);
	assert(!prepared.cameraNavigationAvailable);
	assert(prepared.cameraNavigation.graph().empty());
}

static void testNoDeclaredCameraMapAllowed() {
	FakeResources resources;
	ResolvedRoomTransition t = makeTransition();
	t.targetCameraMap.clear();

	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	Common::String error;
	assert(loader.load("Mp1", t, resources, prepared, error));
	assert(!prepared.cameraMapDeclared);
	assert(!prepared.cameraMapEmpty);
	assert(!prepared.cameraNavigationAvailable);
	assert(prepared.cameraMapPath.empty());
}

static void testActorMapStillRequired() {
	FakeResources resources;
	resources.actorMapEmpty = true;

	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	Common::String error;
	assert(!loader.load("Mp1", makeTransition(), resources,
	                    prepared, error));
	assert(!error.empty());

	resources.actorMapEmpty = false;
	resources.actorMapReadable = false;
	error.clear();
	assert(!loader.load("Mp1", makeTransition(), resources,
	                    prepared, error));
	assert(!error.empty());
}

static void testDeclaredMissingCameraMapIsError() {
	FakeResources resources;
	resources.cameraMapReadable = false;

	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	Common::String error;
	assert(!loader.load("Mp1", makeTransition(), resources,
	                    prepared, error));
	assert(!error.empty());
}

static void testTransactionalHostCommit() {
	FakeResources resources;
	FakeActivation activation;
	RoomNavigationTransitionHost host;
	host.bind("mp1", &resources, &activation);

	Common::String error;
	const ResolvedRoomTransition t = makeTransition();

	assert(host.activateResolvedRoom(t, error));
	assert(error.empty());
	assert(activation.calls == 1);
	assert(host.currentNavigation().roomName == "Room1_2");
	assert(host.currentNavigation().cameraNavigationAvailable);

	// A later target preparation succeeds, but scene activation rejects it.
	// The last successfully committed navigation must survive unchanged.
	activation.accept = false;
	resources.cameraMapEmpty = true;
	assert(!host.activateResolvedRoom(t, error));
	assert(error == "scene activation failed");
	assert(activation.calls == 2);
	assert(host.currentNavigation().roomName == "Room1_2");
	assert(host.currentNavigation().cameraNavigationAvailable);
	assert(!host.currentNavigation().cameraMapEmpty);
}

int main() {
	testPathConstruction();
	testTwoNavigationPlans();
	testRetailEmptyCameraMapAllowed();
	testNoDeclaredCameraMapAllowed();
	testActorMapStillRequired();
	testDeclaredMissingCameraMapIsError();
	testTransactionalHostCommit();
	return 0;
}
