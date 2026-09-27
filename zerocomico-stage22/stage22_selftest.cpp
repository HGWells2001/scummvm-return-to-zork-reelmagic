/* Zero Comico Stage 22 room transition runtime self-test. */

#include <cassert>
#include <cmath>

#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage19/shape_geometry.h"
#include "zerocomico-stage21/room_topology.h"
#include "zerocomico-stage22/room_transition.h"

using namespace ZeroComico;

static bool nearValue(float a, float b) {
	return std::fabs(a - b) < 0.0005f;
}

static const char *kRooms =
	"ge_MainPlace Mp1\n"
	"{\n"
	"Room Room1_1 {\n"
	" Prefix: r11_\n"
	" map: r11_Map00\n"
	" cameramap: r11_MapCam00\n"
	" camera: r11_Camera01\n"
	" cameraspot: r11_Spot01\n"
	" portal: p11_to_12 Room1_2 entry \"vis one\" enabled\n"
	"}\n"
	"Room Room1_2 {\n"
	" Prefix: r12_\n"
	" backgrd: \"r12_Background\"\n"
	" objects: \"r12_Objects\"\n"
	" map: r12_Map00\n"
	" cameramap: r12_MapCam00\n"
	" camera: r12_Camera03\n"
	" cameraspot: r12_Spot03\n"
	" portal: p12_to_11 Room1_1 entry \"vis two\" enabled\n"
	"}\n"
	"}\n";

static const char *kShapes =
	"ge_Shape p11_to_12 Portal\n"
	"A 1 2 3\n"
	"B 4 5 6\n"
	"ge_Shape p12_to_11 Portal\n"
	"A 4 5 6\n"
	"B 1 2 3\n";

static RoomTransitionGraph buildGraph() {
	RoomTopologyDocument rooms;
	RoomTopologyParser roomParser;
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	ShapeGeometryDocument geometry;
	ShapeGeometryParser geometryParser;
	Common::String error;

	assert(roomParser.parse(kRooms, rooms, error));
	assert(shapeParser.parse(kShapes, shapes, error));
	assert(geometryParser.parse(kShapes, geometry, error));

	RoomTransitionGraph graph;
	RoomTransitionGraphBuilder builder;
	builder.build(rooms, shapes, &geometry, graph);
	return graph;
}

class FakeHost : public RoomTransitionRuntimeHost {
public:
	FakeHost() : accept(true), calls(0), sawGeometry(false) {}

	bool activateResolvedRoom(const ResolvedRoomTransition &transition,
	                          Common::String &errorMessage) override {
		++calls;
		last = transition;
		sawGeometry = transition.portalGeometry != nullptr;

		if (!accept) {
			errorMessage = "simulated room load failure";
			return false;
		}

		errorMessage.clear();
		return true;
	}

	bool accept;
	int calls;
	bool sawGeometry;
	ResolvedRoomTransition last;
};

static void testGraphCarriesTargetResources() {
	RoomTransitionGraph graph = buildGraph();
	assert(graph.transitions().size() == 2);

	const ResolvedRoomTransition *transition =
		graph.find("room1_1", "P11_TO_12");
	assert(transition);
	assert(transition->sourceRoom == "Room1_1");
	assert(transition->targetRoom == "Room1_2");
	assert(transition->portalShape == "p11_to_12");

	assert(transition->targetPrefix == "r12_");
	assert(transition->targetBackground == "r12_Background");
	assert(transition->targetObjects == "r12_Objects");
	assert(transition->targetMap == "r12_Map00");
	assert(transition->targetCameraMap == "r12_MapCam00");
	assert(transition->targetCamera == "r12_Camera03");
	assert(transition->targetCameraSpot == "r12_Spot03");

	assert(transition->rawPortalFields.size() == 5);
	assert(transition->portalGeometry);
	assert(transition->portalGeometry->hasNumericA());
	assert(transition->portalGeometry->hasNumericB());
	assert(nearValue(transition->portalGeometry->a.value.x, 1.0f));
	assert(nearValue(transition->portalGeometry->b.value.z, 6.0f));
}

static void testSuccessfulTransitionCommitsRoom() {
	RoomTransitionGraph graph = buildGraph();
	FakeHost host;

	RoomTransitionRuntime runtime;
	runtime.bind(&graph, &host);
	runtime.setCurrentRoom("Room1_1");

	assert(runtime.activatePortal("p11_to_12") == kRoomTransitionDone);
	assert(runtime.currentRoom() == "Room1_2");
	assert(runtime.lastError().empty());
	assert(host.calls == 1);
	assert(host.last.targetMap == "r12_Map00");
	assert(host.last.targetCamera == "r12_Camera03");
	assert(host.sawGeometry);

	assert(runtime.activatePortal("p12_to_11") == kRoomTransitionDone);
	assert(runtime.currentRoom() == "Room1_1");
	assert(host.calls == 2);
}

static void testRejectedTransitionRollsBack() {
	RoomTransitionGraph graph = buildGraph();
	FakeHost host;
	host.accept = false;

	RoomTransitionRuntime runtime;
	runtime.bind(&graph, &host);
	runtime.setCurrentRoom("Room1_1");

	assert(runtime.activatePortal("p11_to_12") ==
	       kRoomTransitionHostRejected);
	assert(runtime.currentRoom() == "Room1_1");
	assert(runtime.lastError() == "simulated room load failure");
	assert(host.calls == 1);
}

static void testUnknownPortalDoesNothing() {
	RoomTransitionGraph graph = buildGraph();
	FakeHost host;

	RoomTransitionRuntime runtime;
	runtime.bind(&graph, &host);
	runtime.setCurrentRoom("Room1_1");

	assert(runtime.activatePortal("not_a_portal") ==
	       kRoomTransitionNotFound);
	assert(runtime.currentRoom() == "Room1_1");
	assert(host.calls == 0);
	assert(!runtime.lastError().empty());
}

static void testAmbiguousPortalExcludedFromGraph() {
	const char *ambiguousRooms =
		"Room A {\n"
		" portal: P B C \"visibility\" on\n"
		"}\n"
		"Room B { }\n"
		"Room C { }\n";

	const char *shapeText =
		"ge_Shape P Portal\n"
		"A 0 0 0\n"
		"B 1 0 0\n";

	RoomTopologyDocument rooms;
	RoomTopologyParser roomParser;
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	ShapeGeometryDocument geometry;
	ShapeGeometryParser geometryParser;
	Common::String error;

	assert(roomParser.parse(ambiguousRooms, rooms, error));
	assert(shapeParser.parse(shapeText, shapes, error));
	assert(geometryParser.parse(shapeText, geometry, error));

	RoomTransitionGraph graph;
	RoomTransitionGraphBuilder builder;
	builder.build(rooms, shapes, &geometry, graph);
	assert(graph.transitions().empty());
}

static void testGeometryIsOptional() {
	RoomTopologyDocument rooms;
	RoomTopologyParser roomParser;
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	Common::String error;

	assert(roomParser.parse(kRooms, rooms, error));
	assert(shapeParser.parse(kShapes, shapes, error));

	RoomTransitionGraph graph;
	RoomTransitionGraphBuilder builder;
	builder.build(rooms, shapes, nullptr, graph);
	assert(graph.transitions().size() == 2);
	assert(graph.transitions()[0].portalGeometry == nullptr);
}

int main() {
	testGraphCarriesTargetResources();
	testSuccessfulTransitionCommitsRoom();
	testRejectedTransitionRollsBack();
	testUnknownPortalDoesNothing();
	testAmbiguousPortalExcludedFromGraph();
	testGeometryIsOptional();
	return 0;
}
