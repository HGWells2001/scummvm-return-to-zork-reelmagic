/* Zero Comico Stage 13 SequenceTable forensic self-test. */

#include <cassert>

#include "zerocomico-stage13/sequence_table.h"

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
	testEmptyRejected();
	return 0;
}
