/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE13_SEQUENCE_TABLE_H
#define ZEROCOMICO_STAGE13_SEQUENCE_TABLE_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

enum SequenceDirectiveKind {
	kSequenceDirectiveUnknown,
	kSequenceDirectiveJacsBanner,
	kSequenceDirectiveFromSeq,
	kSequenceDirectiveStart,
	kSequenceDirectiveBlending,
	kSequenceDirectiveTable,
	kSequenceDirectiveEndSeq,
	kSequenceDirectiveEndSeqAlt,
	kSequenceDirectiveEnd
};

enum SequenceTransitionClass {
	kSequenceTransitionUnknown,
	kSequenceTransitionZeroToOne,
	kSequenceTransitionOneToOne,
	kSequenceTransitionOneToZero
};

struct SequenceDirective {
	SequenceDirectiveKind kind;
	SequenceTransitionClass explicitTransition;
	uint32 lineNumber;
	Common::String keyword;
	Common::String payload;
	Common::String rawLine;

	SequenceDirective() :
		kind(kSequenceDirectiveUnknown),
		explicitTransition(kSequenceTransitionUnknown),
		lineNumber(0) {}
};

struct SequenceOpaqueLine {
	uint32 lineNumber;
	Common::String text;

	SequenceOpaqueLine() : lineNumber(0) {}
};

class SequenceTableDocument {
public:
	void clear();

	bool hasJacsBanner() const { return _hasJacsBanner; }
	const Common::Array<SequenceDirective> &directives() const { return _directives; }
	const Common::Array<SequenceOpaqueLine> &opaqueLines() const { return _opaqueLines; }

	uint32 count(SequenceDirectiveKind kind) const;
	bool containsIdentifier(const Common::String &identifier) const;

private:
	friend class SequenceTableForensicParser;

	bool _hasJacsBanner;
	Common::Array<SequenceDirective> _directives;
	Common::Array<SequenceOpaqueLine> _opaqueLines;
};

/**
 * Conservative line-oriented reader for the retail JACS SequenceTable text.
 *
 * This parser only recognizes tokens proven by japotek3d.dll strings:
 *   blending, table, endseq, end_seq, fromseq, start
 *
 * It also records literal 0>1, 1>1 and 1>0 markers if they are present in
 * input. It never invents animation semantics for opaque lines.
 */
class SequenceTableForensicParser {
public:
	bool parse(const Common::String &decodedText,
	           SequenceTableDocument &out,
	           Common::String &errorMessage) const;
};

const char *sequenceDirectiveName(SequenceDirectiveKind kind);
const char *sequenceTransitionName(SequenceTransitionClass transition);

} // End of namespace ZeroComico

#endif
