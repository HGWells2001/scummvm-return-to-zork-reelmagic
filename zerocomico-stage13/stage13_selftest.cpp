/* Zero Comico Stage 13 SequenceTable forensic self-test. */

#include <cassert>

#include "zerocomico-stage13/sequence_table.h"
#include "zerocomico-stage13/actor_sequence_table.h"

using namespace ZeroComico;

static const char *kSample =
	"//JapoTek Animation Control System (JACS)\r\n"
	"\r\n"
	"walk_loop\r\n"
	"fromseq stop2\r\n"
	"start 0>1 walk_start\r\n"
	"blending 1>1 walk_loop\r\n"
	"table body_leg body_arm\r\n"
	"endseq 1>0 walk_stop\r\n"
	"end_seq\r\n"
	"end\r\n";

static void testParser() {
	SequenceTableDocument doc;
	SequenceTableForensicParser parser;
	Common::String error;

	assert(parser.parse(kSample, doc, error));
	assert(error.empty());
	assert(doc.hasJacsBanner());

	assert(doc.count(kSequenceDirectiveJacsBanner) == 1);
	assert(doc.count(kSequenceDirectiveFromSeq) == 1);
	assert(doc.count(kSequenceDirectiveStart) == 1);
	assert(doc.count(kSequenceDirectiveBlending) == 1);
	assert(doc.count(kSequenceDirectiveTable) == 1);
	assert(doc.count(kSequenceDirectiveEndSeq) == 1);
	assert(doc.count(kSequenceDirectiveEndSeqAlt) == 1);
	assert(doc.count(kSequenceDirectiveEnd) == 1);

	// Blank physical line 2 must still count.
	assert(doc.opaqueLines().size() == 1);
	assert(doc.opaqueLines()[0].lineNumber == 3);
	assert(doc.opaqueLines()[0].text == "walk_loop");

	const Common::Array<SequenceDirective> &d = doc.directives();
	assert(d.size() == 8);
	assert(d[0].kind == kSequenceDirectiveJacsBanner);
	assert(d[0].lineNumber == 1);

	assert(d[1].kind == kSequenceDirectiveFromSeq);
	assert(d[1].lineNumber == 4);
	assert(d[1].payload == "stop2");
	assert(d[1].explicitTransition == kSequenceTransitionUnknown);

	assert(d[2].kind == kSequenceDirectiveStart);
	assert(d[2].lineNumber == 5);
	assert(d[2].explicitTransition == kSequenceTransitionZeroToOne);

	assert(d[3].kind == kSequenceDirectiveBlending);
	assert(d[3].lineNumber == 6);
	assert(d[3].explicitTransition == kSequenceTransitionOneToOne);

	assert(d[5].kind == kSequenceDirectiveEndSeq);
	assert(d[5].lineNumber == 8);
	assert(d[5].explicitTransition == kSequenceTransitionOneToZero);

	assert(doc.containsIdentifier("walk_loop"));
	assert(doc.containsIdentifier("STOP2"));
	assert(doc.containsIdentifier("body_arm"));
	assert(!doc.containsIdentifier("walk"));
}

static void testInlineCommentsAndQuotes() {
	const char *text =
		"start foo // start in comment must not create another directive\n"
		"opaque \"// not comment\" token\n"
		"// endseq only comment\n";

	SequenceTableDocument doc;
	SequenceTableForensicParser parser;
	Common::String error;
	assert(parser.parse(text, doc, error));

	assert(doc.count(kSequenceDirectiveStart) == 1);
	assert(doc.count(kSequenceDirectiveEndSeq) == 0);
	assert(doc.opaqueLines().size() == 1);
	assert(doc.opaqueLines()[0].lineNumber == 2);
	assert(doc.containsIdentifier("token"));
}

static void testExactAnjClipCorrelation() {
	const char *text =
		"walk_loop\n"
		"start 0>1 gio_walk_start\n"
		"blending 1>1 gio_walk_loop\n"
		"endseq 1>0 gio_walk_stop\n"
		"opaque gio_idle\n";

	SequenceTableDocument doc;
	SequenceTableForensicParser parser;
	Common::String error;
	assert(parser.parse(text, doc, error));

	Common::Array<AnimationClip> clips;
	AnimationClip clip;
	clip.name = "gio_walk_start"; clips.push_back(clip);
	clip.name = "gio_walk_loop"; clips.push_back(clip);
	clip.name = "gio_walk_stop"; clips.push_back(clip);
	clip.name = "gio_idle"; clips.push_back(clip);
	clip.name = "walk"; clips.push_back(clip);
	clip.name = "not_referenced"; clips.push_back(clip);

	SequenceClipCorrelation correlation;
	SequenceClipCorrelator correlator;
	correlator.correlate(doc, clips, correlation);

	assert(correlation.referencedClips.size() == 4);
	assert(correlation.unreferencedClips.size() == 2);
	assert(correlation.references.size() == 4);

	assert(correlation.references[0].clipName == "gio_walk_start");
	assert(correlation.references[0].directiveKind == kSequenceDirectiveStart);
	assert(correlation.references[0].explicitTransition ==
	       kSequenceTransitionZeroToOne);

	assert(correlation.references[1].clipName == "gio_walk_loop");
	assert(correlation.references[1].directiveKind ==
	       kSequenceDirectiveBlending);
	assert(correlation.references[1].explicitTransition ==
	       kSequenceTransitionOneToOne);

	assert(correlation.references[2].clipName == "gio_walk_stop");
	assert(correlation.references[2].directiveKind ==
	       kSequenceDirectiveEndSeq);
	assert(correlation.references[2].explicitTransition ==
	       kSequenceTransitionOneToZero);

	assert(correlation.references[3].clipName == "gio_idle");
	assert(correlation.references[3].directiveKind ==
	       kSequenceDirectiveUnknown);
}

static void testEmptyRejected() {
	SequenceTableDocument doc;
	SequenceTableForensicParser parser;
	Common::String error;
	assert(!parser.parse("", doc, error));
	assert(!error.empty());
}

int main() {
	testParser();
	testInlineCommentsAndQuotes();
	testExactAnjClipCorrelation();
	testEmptyRejected();
	return 0;
}
