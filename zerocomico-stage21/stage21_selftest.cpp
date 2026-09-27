/* Zero Comico Stage 21 room/portal topology self-test. */

#include <cassert>

#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage21/room_topology.h"

using namespace ZeroComico;

static const char *kRooms =
	"ge_MainPlace Mp1\r\n"
	"{\r\n"
	"  Room Room1_1\r\n"
	"  {\r\n"
	"    Prefix: r11_\r\n"
	"    backgrd: \"r11_Camera01\"\r\n"
	"    objects: \"r11_objects\"\r\n"
	"    map: r11_Map00\r\n"
	"    cameramap: r11_MapCam00\r\n"
	"    camera: r11_Camera01\r\n"
	"    cameraspot: r11_Spot01\r\n"
	"    portal: r11_exit Room1_2 entry \"vis set one\" enabled\r\n"
	"  }\r\n"
	"  Room Room1_2 {\r\n"
	"    Prefix: r12_\r\n"
	"    map: r12_Map00\r\n"
	"    portal: r12_exit Room1_1 entry \"vis set two\" enabled\r\n"
	"  }\r\n"
	"}\r\n";

static const char *kShapes =
	"ge_Shape r11_exit Portal\n"
	"ge_Shape r12_exit Portal\n"
	"ge_Shape SomePosition Position\n";

static void testRoomFields() {
	RoomTopologyDocument doc;
	RoomTopologyParser parser;
	Common::String error;
	assert(parser.parse(kRooms, doc, error));
	assert(error.empty());
	assert(doc.rooms().size() == 2);

	const ExtendedRoomRecord *room = doc.room("room1_1");
	assert(room);
	assert(room->lineNumber == 3);
	assert(room->prefix == "r11_");
	assert(room->background == "r11_Camera01");
	assert(room->objects == "r11_objects");
	assert(room->map == "r11_Map00");
	assert(room->cameraMap == "r11_MapCam00");
	assert(room->camera == "r11_Camera01");
	assert(room->cameraSpot == "r11_Spot01");
	assert(room->portals.size() == 1);

	const RoomPortalRecord &portal = room->portals[0];
	assert(portal.fields.size() == 5);
	assert(portal.fields[0] == "r11_exit");
	assert(portal.fields[1] == "Room1_2");
	assert(portal.fields[2] == "entry");
	assert(portal.fields[3] == "vis set one");
	assert(portal.fields[4] == "enabled");
}

static void testUniqueTopologyResolution() {
	RoomTopologyDocument rooms;
	RoomTopologyParser roomParser;
	Common::String error;
	assert(roomParser.parse(kRooms, rooms, error));

	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	assert(shapeParser.parse(kShapes, shapes, error));

	Common::Array<RoomPortalResolution> resolutions;
	RoomPortalTopologyResolver resolver;
	resolver.resolve(rooms, shapes, resolutions);
	assert(resolutions.size() == 2);

	const RoomPortalResolution &first = resolutions[0];
	assert(first.ownerRoom == "Room1_1");
	assert(first.hasUniqueRoom());
	assert(first.matchingRooms[0] == "Room1_2");
	assert(first.hasUniquePortalShape());
	assert(first.matchingPortalShapes[0] == "r11_exit");

	const RoomPortalResolution &second = resolutions[1];
	assert(second.ownerRoom == "Room1_2");
	assert(second.hasUniqueRoom());
	assert(second.matchingRooms[0] == "Room1_1");
	assert(second.hasUniquePortalShape());
	assert(second.matchingPortalShapes[0] == "r12_exit");
}

static void testAmbiguousRoomRemainsAmbiguous() {
	const char *ambiguous =
		"ge_MainPlace Mp1\n"
		"{\n"
		"Room A {\n"
		"portal: P B C \"vis\" on\n"
		"}\n"
		"Room B { }\n"
		"Room C { }\n"
		"}\n";

	const char *shapeText =
		"ge_Shape P Portal\n";

	RoomTopologyDocument rooms;
	RoomTopologyParser roomParser;
	Common::String error;
	assert(roomParser.parse(ambiguous, rooms, error));

	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	assert(shapeParser.parse(shapeText, shapes, error));

	Common::Array<RoomPortalResolution> resolutions;
	RoomPortalTopologyResolver resolver;
	resolver.resolve(rooms, shapes, resolutions);
	assert(resolutions.size() == 1);
	assert(resolutions[0].matchingRooms.size() == 2);
	assert(!resolutions[0].hasUniqueRoom());
	assert(resolutions[0].hasUniquePortalShape());
}

static void testMalformedPortalRejected() {
	const char *bad =
		"Room R {\n"
		"portal: only four fields here\n"
		"}\n";

	RoomTopologyDocument doc;
	RoomTopologyParser parser;
	Common::String error;
	assert(!parser.parse(bad, doc, error));
	assert(!error.empty());
}

static void testQuotedCommentPreserved() {
	const char *text =
		"Room R {\n"
		"portal: P Other E \"keep // inside\" on // real comment\n"
		"}\n"
		"Room Other { }\n";

	RoomTopologyDocument doc;
	RoomTopologyParser parser;
	Common::String error;
	assert(parser.parse(text, doc, error));
	const ExtendedRoomRecord *r = doc.room("R");
	assert(r && r->portals.size() == 1);
	assert(r->portals[0].fields[3] == "keep // inside");
}

int main() {
	testRoomFields();
	testUniqueTopologyResolution();
	testAmbiguousRoomRemainsAmbiguous();
	testMalformedPortalRejected();
	testQuotedCommentPreserved();
	return 0;
}
