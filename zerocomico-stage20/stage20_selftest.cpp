/* Zero Comico Stage 20 numeric world-placement self-test. */

#include <cassert>
#include <cmath>

#include "zerocomico-stage20/character_start_placement.h"
#include "zerocomico-stage20/geometry_world_opcode_service.h"

using namespace ZeroComico;

static bool nearValue(float a, float b) {
	return std::fabs(a - b) < 0.0005f;
}

static const char *kChar =
	"ge_Character Pacman\n"
	"{\n"
	"  AnimSet Walk pac_pacman\n"
	"}\n"
	"SetCharPos_Vector Pacman r12_Start_Pacman\n";

static const char *kShape =
	"ge_Shape r12_Start_Pacman Position\n"
	"{\n"
	"  A 10.5 20 -3\n"
	"  B 11.5 20 -3\n"
	"}\n"
	"ge_Shape Door12 Portal\n"
	"{\n"
	"  A 0 0 0\n"
	"  B 1 0 0\n"
	"}\n";

class FakeResources : public GameplayResourceHost {
public:
	bool readDecodedText(const Common::Path &path, Common::String &text) override {
		if (path.toString() == "Mp1/gameplay/char.isc") {
			text = kChar;
			return true;
		}
		if (path.toString() == "Mp1/gameplay/Shape.shp") {
			text = kShape;
			return true;
		}
		return false;
	}

	bool readPlainText(const Common::Path &, Common::String &) override {
		return false;
	}
};

class FakeWorld : public GeometryWorldRuntimeHost {
public:
	FakeWorld() :
		placed(false),
		hasNumericA(false),
		hasNumericB(false) {
	}

	bool placeCharacterAtPositionGeometry(
			const Common::String &characterName,
			const ShapeDefinition &positionShape,
			const ShapeGeometryRecord &geometry) override {
		if (!characterName.equalsIgnoreCase("Pacman"))
			return false;
		if (!positionShape.name.equalsIgnoreCase("r12_Start_Pacman"))
			return false;

		placed = true;
		character = characterName;
		helper = positionShape.name;
		hasNumericA = geometry.hasNumericA();
		hasNumericB = geometry.hasNumericB();
		if (hasNumericA)
			a = geometry.a.value;
		if (hasNumericB)
			b = geometry.b.value;
		return true;
	}

	bool placed;
	bool hasNumericA;
	bool hasNumericB;
	Common::String character;
	Common::String helper;
	ShapeVec3 a;
	ShapeVec3 b;
};

static void testTransactionalStartPlacement() {
	FakeResources resources;
	CharacterStartPlacement placement;
	CharacterStartPlacementLoader loader;
	Common::String error;

	assert(loader.load("mp1", "Pacman", resources, placement, error));
	assert(error.empty());
	assert(placement.valid());
	assert(placement.binding.resolution.helperName == "r12_Start_Pacman");
	assert(placement.resolvedGeometry.helperName == "r12_Start_Pacman");
	assert(placement.resolvedGeometry.geometry);
	assert(placement.resolvedGeometry.geometry->hasNumericA());
	assert(placement.resolvedGeometry.geometry->hasNumericB());

	const ShapeGeometryRecord *g = placement.resolvedGeometry.geometry;
	assert(nearValue(g->a.value.x, 10.5f));
	assert(nearValue(g->a.value.y, 20.0f));
	assert(nearValue(g->a.value.z, -3.0f));
	assert(nearValue(g->b.value.x, 11.5f));
}

static void testGeometryAwareOpcode() {
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	ShapeGeometryDocument geometry;
	ShapeGeometryParser geometryParser;
	Common::String error;
	assert(shapeParser.parse(kShape, shapes, error));
	assert(geometryParser.parse(kShape, geometry, error));

	FakeWorld world;
	GeometryWorldOpcodeService service;
	service.bind(&shapes, &geometry, &world);

	Common::Array<Common::String> args;
	args.push_back("Pacman");
	args.push_back("r12_Start_Pacman");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalDone);
	assert(world.placed);
	assert(world.character == "Pacman");
	assert(world.helper == "r12_Start_Pacman");
	assert(world.hasNumericA);
	assert(world.hasNumericB);
	assert(nearValue(world.a.x, 10.5f));
	assert(nearValue(world.b.x, 11.5f));

	Common::Array<Common::String> none;
	assert(service.executeObjectHandlerOpcode("portals_on", none) ==
	       kObjectHandlerExternalUnhandled);
	assert(service.executeObjectHandlerOpcode("portals_off", none) ==
	       kObjectHandlerExternalUnhandled);
}

static void testWrongShapeRejected() {
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	ShapeGeometryDocument geometry;
	ShapeGeometryParser geometryParser;
	Common::String error;
	assert(shapeParser.parse(kShape, shapes, error));
	assert(geometryParser.parse(kShape, geometry, error));

	FakeWorld world;
	GeometryWorldOpcodeService service;
	service.bind(&shapes, &geometry, &world);

	Common::Array<Common::String> args;
	args.push_back("Pacman");
	args.push_back("Door12");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalUnhandled);
	assert(!world.placed);
	assert(!service.lastError().empty());
}

static void testMissingGeometryRejected() {
	const char *symbolicOnly =
		"ge_Shape P Position\n";

	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	Common::String error;
	assert(shapeParser.parse(symbolicOnly, shapes, error));

	ShapeGeometryDocument emptyGeometry;
	FakeWorld world;
	GeometryWorldOpcodeService service;
	service.bind(&shapes, &emptyGeometry, &world);

	Common::Array<Common::String> args;
	args.push_back("Pacman");
	args.push_back("P");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalUnhandled);
	assert(!world.placed);
	assert(!service.lastError().empty());
}

static void testBadArity() {
	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	ShapeGeometryDocument geometry;
	ShapeGeometryParser geometryParser;
	Common::String error;
	assert(shapeParser.parse(kShape, shapes, error));
	assert(geometryParser.parse(kShape, geometry, error));

	FakeWorld world;
	GeometryWorldOpcodeService service;
	service.bind(&shapes, &geometry, &world);

	Common::Array<Common::String> args;
	args.push_back("Pacman");
	assert(service.executeObjectHandlerOpcode("SetCharPos_Vector", args) ==
	       kObjectHandlerExternalBadArguments);
}

int main() {
	testTransactionalStartPlacement();
	testGeometryAwareOpcode();
	testWrongShapeRejected();
	testMissingGeometryRejected();
	testBadArity();
	return 0;
}
