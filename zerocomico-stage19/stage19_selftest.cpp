/* Zero Comico Stage 19 Shape.shp geometry self-test. */

#include <cassert>
#include <cmath>

#include "zerocomico-stage19/shape_geometry.h"

using namespace ZeroComico;

static bool nearValue(float a, float b) {
	return std::fabs(a - b) < 0.0005f;
}

static const char *kShapes =
	"// retail-style shape declarations\r\n"
	"ge_Shape r12_Start_Pacman Position\r\n"
	"{\r\n"
	"  A 12.5 -3 4.25 // numeric endpoint\r\n"
	"  B -1.5e1 +2.0 .5\r\n"
	"}\r\n"
	"ge_Shape Door Portal\r\n"
	"{\r\n"
	"  A portal_a token two\r\n"
	"  B 1 2\r\n"
	"}\r\n"
	"ge_Polygon Floor\r\n"
	"{\r\n"
	"  A 100 200 300\r\n"
	"}\r\n"
	"ge_Shape Range01 Range\r\n"
	"A 0 0 0\r\n"
	"B 5 0 5\r\n";

static void testNumericPosition() {
	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(parser.parse(kShapes, doc, error));
	assert(error.empty());
	assert(doc.shapes().size() == 3);

	const ShapeGeometryRecord *start = doc.shape("r12_start_pacman");
	assert(start);
	assert(start->kind == kShapeDefinitionPosition);
	assert(start->lineNumber == 2);
	assert(start->hasNumericA());
	assert(start->hasNumericB());
	assert(start->a.lineNumber == 4);
	assert(start->b.lineNumber == 5);

	assert(nearValue(start->a.value.x, 12.5f));
	assert(nearValue(start->a.value.y, -3.0f));
	assert(nearValue(start->a.value.z, 4.25f));

	assert(nearValue(start->b.value.x, -15.0f));
	assert(nearValue(start->b.value.y, 2.0f));
	assert(nearValue(start->b.value.z, 0.5f));
}

static void testOpaqueValuesPreserved() {
	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(parser.parse(kShapes, doc, error));

	const ShapeGeometryRecord *door = doc.shape("DOOR");
	assert(door);
	assert(door->kind == kShapeDefinitionPortal);
	assert(door->hasA && !door->a.numeric);
	assert(door->a.rawValues.size() == 3);
	assert(door->a.rawValues[0] == "portal_a");
	assert(door->hasB && !door->b.numeric);
	assert(door->b.rawValues.size() == 2);
}

static void testPolygonDoesNotLeakIntoPreviousShape() {
	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(parser.parse(kShapes, doc, error));

	const ShapeGeometryRecord *door = doc.shape("Door");
	assert(door);
	assert(door->a.rawValues[0] == "portal_a");

	const ShapeGeometryRecord *range = doc.shape("Range01");
	assert(range);
	assert(range->hasNumericA());
	assert(range->hasNumericB());
	assert(nearValue(range->b.value.x, 5.0f));
	assert(nearValue(range->b.value.z, 5.0f));
}

static void testStartGeometryResolution() {
	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(parser.parse(kShapes, doc, error));

	CharacterStartGeometry start;
	assert(resolveCharacterStartGeometry(
		"Pacman", "r12_Start_Pacman", doc, start, error));
	assert(error.empty());
	assert(start.valid());
	assert(start.characterName == "Pacman");
	assert(start.helperName == "r12_Start_Pacman");
	assert(start.geometry->hasNumericA());
	assert(start.geometry->hasNumericB());

	CharacterStartGeometry bad;
	assert(!resolveCharacterStartGeometry(
		"Pacman", "Door", doc, bad, error));
	assert(!error.empty());
}

static void testDuplicateEndpointRejected() {
	const char *bad =
		"ge_Shape P Position\n"
		"A 1 2 3\n"
		"A 4 5 6\n";

	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(!parser.parse(bad, doc, error));
	assert(!error.empty());
}

static void testMalformedNumberStaysOpaque() {
	const char *text =
		"ge_Shape P Position\n"
		"A 1.0 2x 3.0\n";

	ShapeGeometryDocument doc;
	ShapeGeometryParser parser;
	Common::String error;
	assert(parser.parse(text, doc, error));

	const ShapeGeometryRecord *p = doc.shape("P");
	assert(p);
	assert(p->hasA);
	assert(!p->a.numeric);
	assert(p->a.rawValues.size() == 3);
}

int main() {
	testNumericPosition();
	testOpaqueValuesPreserved();
	testPolygonDoesNotLeakIntoPreviousShape();
	testStartGeometryResolution();
	testDuplicateEndpointRejected();
	testMalformedNumberStaysOpaque();
	return 0;
}
