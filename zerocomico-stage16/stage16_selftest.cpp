/* Zero Comico Stage 16 character start binding self-test. */

#include <cassert>

#include "zerocomico-stage16/character_start_binding.h"

using namespace ZeroComico;

static const char *kChar =
	"ge_Character Giovanni\n"
	"{\n"
	"  AnimSet Walk gio_body\n"
	"}\n"
	"SetCharPos_Vector Giovanni R12pos\n";

static const char *kShape =
	"ge_Shape R12pos Position\n"
	"ge_Shape Door12 Portal\n"
	"ge_Vector SomeVector\n";

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

static void testShapeParser() {
	ShapeScriptDocument shapes;
	ShapeScriptParser parser;
	Common::String error;
	assert(parser.parse(kShape, shapes, error));
	assert(error.empty());
	assert(shapes.shapes().size() == 2);
	assert(shapes.vectors().size() == 1);

	const ShapeDefinition *pos = shapes.shape("r12POS");
	assert(pos);
	assert(pos->kind == kShapeDefinitionPosition);

	const ShapeDefinition *portal = shapes.shape("door12");
	assert(portal);
	assert(portal->kind == kShapeDefinitionPortal);
}

static void testDirectResolution() {
	CharacterScriptDocument chars;
	CharacterScriptParser charParser;
	Common::String error;
	assert(charParser.parse(kChar, chars, error));

	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	assert(shapeParser.parse(kShape, shapes, error));

	CharacterStartShapeResolution resolved;
	assert(resolveCharacterStartShape(
		chars, shapes, "Giovanni", resolved, error));
	assert(resolved.valid());
	assert(resolved.helperName == "R12pos");
	assert(resolved.shape->name == "R12pos");
	assert(resolved.shape->kind == kShapeDefinitionPosition);
}

static void testLoader() {
	FakeResources resources;
	CharacterStartBinding binding;
	CharacterStartBindingLoader loader;
	Common::String error;

	assert(loader.load("mp1", "Giovanni", resources, binding, error));
	assert(error.empty());
	assert(binding.valid());
	assert(binding.resolution.helperName == "R12pos");
	assert(gameplayShapeScriptPath("MP1").toString() ==
	       "Mp1/gameplay/Shape.shp");
}

static void testWrongShapeTypeRejected() {
	const char *badShape =
		"ge_Shape R12pos Portal\n";

	CharacterScriptDocument chars;
	CharacterScriptParser charParser;
	Common::String error;
	assert(charParser.parse(kChar, chars, error));

	ShapeScriptDocument shapes;
	ShapeScriptParser shapeParser;
	assert(shapeParser.parse(badShape, shapes, error));

	CharacterStartShapeResolution resolved;
	assert(!resolveCharacterStartShape(
		chars, shapes, "Giovanni", resolved, error));
	assert(!error.empty());
}

int main() {
	testShapeParser();
	testDirectResolution();
	testLoader();
	testWrongShapeTypeRejected();
	return 0;
}
