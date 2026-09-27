/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage13/sequence_table.h"

namespace ZeroComico {

namespace {

static bool isIdentifierChar(char c) {
	return (c >= 'a' && c <= 'z') ||
	       (c >= 'A' && c <= 'Z') ||
	       (c >= '0' && c <= '9') ||
	       c == '_';
}

static bool findWordIgnoreCase(const Common::String &line,
                               const char *word,
                               uint32 &offset) {
	Common::String loweredLine = line;
	Common::String loweredWord(word);
	loweredLine.toLowercase();
	loweredWord.toLowercase();

	if (loweredWord.empty() || loweredLine.size() < loweredWord.size())
		return false;

	for (uint32 i = 0; i + loweredWord.size() <= loweredLine.size(); ++i) {
		bool equal = true;
		for (uint32 n = 0; n < loweredWord.size(); ++n) {
			if (loweredLine[i + n] != loweredWord[n]) {
				equal = false;
				break;
			}
		}
		if (!equal)
			continue;

		const bool leftOk = (i == 0) || !isIdentifierChar(loweredLine[i - 1]);
		const uint32 after = i + loweredWord.size();
		const bool rightOk = (after == loweredLine.size()) ||
		                     !isIdentifierChar(loweredLine[after]);
		if (leftOk && rightOk) {
			offset = i;
			return true;
		}
	}
	return false;
}

static Common::String payloadAfter(const Common::String &line,
                                   uint32 keywordOffset,
                                   uint32 keywordLength) {
	uint32 start = keywordOffset + keywordLength;
	while (start < line.size() &&
	       (line[start] == ' ' || line[start] == '\t' ||
	        line[start] == ':' || line[start] == '=')) {
		++start;
	}

	Common::String result = line.substr(start);
	result.trim();
	return result;
}

static SequenceTransitionClass explicitTransitionIn(const Common::String &line) {
	Common::String compact;
	for (uint32 i = 0; i < line.size(); ++i) {
		const char c = line[i];
		if (c != ' ' && c != '\t')
			compact += c;
	}

	if (compact.contains("0>1"))
		return kSequenceTransitionZeroToOne;
	if (compact.contains("1>1"))
		return kSequenceTransitionOneToOne;
	if (compact.contains("1>0"))
		return kSequenceTransitionOneToZero;
	return kSequenceTransitionUnknown;
}

static bool classifyDirective(const Common::String &code,
                              SequenceDirective &directive) {
	struct KnownWord {
		const char *word;
		SequenceDirectiveKind kind;
	};

	// Longer / more specific forms first.
	static const KnownWord kKnown[] = {
		{"end_seq", kSequenceDirectiveEndSeqAlt},
		{"endseq", kSequenceDirectiveEndSeq},
		{"fromseq", kSequenceDirectiveFromSeq},
		{"blending", kSequenceDirectiveBlending},
		{"table", kSequenceDirectiveTable},
		{"start", kSequenceDirectiveStart},
		{"end", kSequenceDirectiveEnd}
	};

	for (uint32 i = 0; i < ARRAYSIZE(kKnown); ++i) {
		uint32 offset = 0;
		if (!findWordIgnoreCase(code, kKnown[i].word, offset))
			continue;

		directive.kind = kKnown[i].kind;
		directive.keyword = kKnown[i].word;
		directive.payload = payloadAfter(
			code, offset, Common::String(kKnown[i].word).size());
		directive.explicitTransition = explicitTransitionIn(code);
		return true;
	}
	return false;
}

static Common::String stripInlineComment(const Common::String &line,
                                         Common::String &comment) {
	comment.clear();

	bool inQuote = false;
	for (uint32 i = 0; i + 1 < line.size(); ++i) {
		if (line[i] == '"')
			inQuote = !inQuote;
		if (!inQuote && line[i] == '/' && line[i + 1] == '/') {
			comment = line.substr(i);
			Common::String code = line.substr(0, i);
			code.trim();
			return code;
		}
	}

	Common::String code = line;
	code.trim();
	return code;
}

static bool isExactIdentifierToken(const Common::String &line,
                                   const Common::String &identifier) {
	if (identifier.empty())
		return false;

	for (uint32 i = 0; i < line.size();) {
		while (i < line.size() && !isIdentifierChar(line[i]))
			++i;
		const uint32 begin = i;
		while (i < line.size() && isIdentifierChar(line[i]))
			++i;
		if (i <= begin)
			continue;

		const Common::String token = line.substr(begin, i - begin);
		if (token.equalsIgnoreCase(identifier))
			return true;
	}
	return false;
}

} // namespace

void SequenceTableDocument::clear() {
	_hasJacsBanner = false;
	_directives.clear();
	_opaqueLines.clear();
}

uint32 SequenceTableDocument::count(SequenceDirectiveKind kind) const {
	uint32 result = 0;
	for (uint32 i = 0; i < _directives.size(); ++i) {
		if (_directives[i].kind == kind)
			++result;
	}
	return result;
}

bool SequenceTableDocument::containsIdentifier(
		const Common::String &identifier) const {
	for (uint32 i = 0; i < _directives.size(); ++i) {
		if (isExactIdentifierToken(_directives[i].rawLine, identifier))
			return true;
	}
	for (uint32 i = 0; i < _opaqueLines.size(); ++i) {
		if (isExactIdentifierToken(_opaqueLines[i].text, identifier))
			return true;
	}
	return false;
}

bool SequenceTableForensicParser::parse(const Common::String &decodedText,
                                        SequenceTableDocument &out,
                                        Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decodedText.empty()) {
		errorMessage = "SequenceTable text is empty";
		return false;
	}

	uint32 lineNumber = 1;
	uint32 lineStart = 0;

	while (lineStart <= decodedText.size()) {
		uint32 lineEnd = lineStart;
		while (lineEnd < decodedText.size() &&
		       decodedText[lineEnd] != '\r' &&
		       decodedText[lineEnd] != '\n') {
			++lineEnd;
		}

		Common::String raw = decodedText.substr(lineStart, lineEnd - lineStart);

		// Remove UTF-8 BOM if it survived upstream decoding.
		if (lineNumber == 1 && raw.size() >= 3 &&
		    (byte)raw[0] == 0xEF && (byte)raw[1] == 0xBB &&
		    (byte)raw[2] == 0xBF) {
			raw = raw.substr(3);
		}

		Common::String trimmed = raw;
		trimmed.trim();
		if (!trimmed.empty()) {
			Common::String comment;
			Common::String code = stripInlineComment(raw, comment);

			Common::String bannerProbe = comment.empty() ? trimmed : comment;
			Common::String lowerBanner = bannerProbe;
			lowerBanner.toLowercase();
			if (lowerBanner.contains("japotek animation control system") ||
			    lowerBanner.contains("(jacs)")) {
				SequenceDirective banner;
				banner.kind = kSequenceDirectiveJacsBanner;
				banner.lineNumber = lineNumber;
				banner.keyword = "JACS";
				banner.rawLine = raw;
				out._directives.push_back(banner);
				out._hasJacsBanner = true;
			}

			if (!code.empty()) {
				SequenceDirective directive;
				directive.lineNumber = lineNumber;
				directive.rawLine = raw;
				if (classifyDirective(code, directive)) {
					out._directives.push_back(directive);
				} else {
					SequenceOpaqueLine opaque;
					opaque.lineNumber = lineNumber;
					opaque.text = code;
					out._opaqueLines.push_back(opaque);
				}
			}
		}

		if (lineEnd >= decodedText.size())
			break;

		if (decodedText[lineEnd] == '\r' &&
		    lineEnd + 1 < decodedText.size() &&
		    decodedText[lineEnd + 1] == '\n') {
			lineStart = lineEnd + 2;
		} else {
			lineStart = lineEnd + 1;
		}
		++lineNumber;
	}

	if (out._directives.empty() && out._opaqueLines.empty()) {
		errorMessage = "SequenceTable text has no readable content";
		return false;
	}

	return true;
}

const char *sequenceDirectiveName(SequenceDirectiveKind kind) {
	switch (kind) {
	case kSequenceDirectiveJacsBanner: return "jacs-banner";
	case kSequenceDirectiveFromSeq: return "fromseq";
	case kSequenceDirectiveStart: return "start";
	case kSequenceDirectiveBlending: return "blending";
	case kSequenceDirectiveTable: return "table";
	case kSequenceDirectiveEndSeq: return "endseq";
	case kSequenceDirectiveEndSeqAlt: return "end_seq";
	case kSequenceDirectiveEnd: return "end";
	default: return "unknown";
	}
}

const char *sequenceTransitionName(SequenceTransitionClass transition) {
	switch (transition) {
	case kSequenceTransitionZeroToOne: return "0>1";
	case kSequenceTransitionOneToOne: return "1>1";
	case kSequenceTransitionOneToZero: return "1>0";
	default: return "unknown";
	}
}

} // End of namespace ZeroComico
