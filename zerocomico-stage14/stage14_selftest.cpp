/* Zero Comico Stage 14 character AnimSet self-test. */

#include <cassert>

#include "zerocomico-stage13/sequence_table.h"
#include "zerocomico-stage14/animset_motion_profile.h"
#include "zerocomico-stage14/character_animset.h"

using namespace ZeroComico;

static const char *kCharacterScript =
	"// retail-style character script\r\n"
	"\r\n"
	"ge_Character Pacman\r\n"
	"{\r\n"
	"  AnimSet Walk pac_pacman\r\n"
	"  standby: pac_stay pac_after_run\r\n"
	"  Blend: pac_blend_start pac_blend_end\r\n"
	"  turn: pac_turn_left pac_turn_right\r\n"
	"  walk: pac_walk_seq pac_run_seq\r\n"
	"  special_standby: pac_idle_alt\r\n"
	"}\r\n"
	"\r\n"
	"SetCharPos_Vector Pacman r12_Start_Pacman\r\n"
	"ge_Character Operaio {\r\n"
	"  AnimSet Work op_worker\r\n"
	"  standby: op_idle\r\n"
	"}\r\n";

static const char *kSequenceSample =
	"//JapoTek Animation Control System (JACS)\n"
	"pac_walk_seq\n"
	"fromseq pac_walk_seq\n"
	"pac_run_seq\n"
	"endseq\n";

static void testCharacterParsing() {
	CharacterScriptDocument doc;
	CharacterScriptParser parser;
	Common::String error;
	assert(parser.parse(kCharacterScript, doc, error));
	assert(error.empty());
	assert(doc.characters().size() == 2);
	assert(doc.startVectors().size() == 1);

	const CharacterDefinition *pacman = doc.character("Pacman");
	assert(pacman);
	assert(pacman->lineNumber == 3);
	assert(pacman->animSets.size() == 1);

	const CharacterAnimSet *walk = pacman->animSet("walk");
	assert(walk);
	assert(walk->lineNumber == 5);
	assert(walk->entity == "pac_pacman");

	const CharacterAnimSetField *standby = walk->field("STANDBY");
	assert(standby);
	assert(standby->lineNumber == 6);
	assert(standby->values.size() == 2);
	assert(standby->values[0] == "pac_stay");
	assert(standby->values[1] == "pac_after_run");

	const CharacterAnimSetField *walkField = walk->field("walk");
	assert(walkField);
	assert(walkField->lineNumber == 9);
	assert(walkField->values.size() == 2);
	assert(walkField->values[0] == "pac_walk_seq");
	assert(walkField->values[1] == "pac_run_seq");

	const CharacterStartVector *start = doc.startVector("pacman");
	assert(start);
	assert(start->lineNumber == 13);
	assert(start->helperName == "r12_Start_Pacman");

	const CharacterDefinition *worker = doc.character("Operaio");
	assert(worker);
	assert(worker->animSet("Work"));
}

static void testMotionProfile() {
	CharacterScriptDocument doc;
	CharacterScriptParser parser;
	Common::String error;
	assert(parser.parse(kCharacterScript, doc, error));

	const CharacterAnimSet *walk = doc.character("Pacman")->animSet("Walk");
	assert(walk);

	AnimSetMotionProfile profile;
	assert(buildAnimSetMotionProfile(*walk, profile, error));
	assert(error.empty());

	assert(profile.standbyAnimation == "pac_stay");
	assert(profile.standbyAfterRunAnimation == "pac_after_run");
	assert(profile.blendStartAnimation == "pac_blend_start");
	assert(profile.blendEndAnimation == "pac_blend_end");
	assert(profile.turnLeftAnimation == "pac_turn_left");
	assert(profile.turnRightAnimation == "pac_turn_right");
	assert(profile.walkSequence == "pac_walk_seq");
	assert(profile.runSequence == "pac_run_seq");
}

static void testSequenceCorrelation() {
	CharacterScriptDocument chars;
	CharacterScriptParser charParser;
	Common::String error;
	assert(charParser.parse(kCharacterScript, chars, error));

	AnimSetMotionProfile profile;
	assert(buildAnimSetMotionProfile(
		*chars.character("Pacman")->animSet("Walk"), profile, error));

	SequenceTableDocument seq;
	SequenceTableForensicParser seqParser;
	assert(seqParser.parse(kSequenceSample, seq, error));

	const AnimSetSequenceEvidence evidence =
		correlateMotionSequences(profile, seq);
	assert(evidence.walkSequenceMentioned);
	assert(evidence.runSequenceMentioned);
}

static void testExcessPairRejected() {
	const char *bad =
		"ge_Character Test {\n"
		"AnimSet Walk body\n"
		"walk: a b c\n"
		"}\n";

	CharacterScriptDocument doc;
	CharacterScriptParser parser;
	Common::String error;
	assert(parser.parse(bad, doc, error));

	AnimSetMotionProfile profile;
	assert(!buildAnimSetMotionProfile(
		*doc.character("Test")->animSet("Walk"), profile, error));
	assert(!error.empty());
}

int main() {
	testCharacterParsing();
	testMotionProfile();
	testSequenceCorrelation();
	testExcessPairRejected();
	return 0;
}
