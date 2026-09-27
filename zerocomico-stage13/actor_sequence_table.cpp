/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage13/actor_sequence_table.h"

#include "zerocomico-stage9/resource_decoder.h"

namespace ZeroComico {

namespace {

static bool identifierChar(char c) {
	return (c >= 'a' && c <= 'z') ||
	       (c >= 'A' && c <= 'Z') ||
	       (c >= '0' && c <= '9') ||
	       c == '_';
}

static bool lineContainsIdentifier(const Common::String &line,
                                   const Common::String &identifier) {
	if (identifier.empty())
		return false;

	for (uint32 i = 0; i < line.size();) {
		while (i < line.size() && !identifierChar(line[i]))
			++i;
		const uint32 begin = i;
		while (i < line.size() && identifierChar(line[i]))
			++i;
		if (i <= begin)
			continue;

		if (line.substr(begin, i - begin).equalsIgnoreCase(identifier))
			return true;
	}
	return false;
}

static bool containsStringIgnoreCase(const Common::Array<Common::String> &list,
                                     const Common::String &value) {
	for (uint32 i = 0; i < list.size(); ++i) {
		if (list[i].equalsIgnoreCase(value))
			return true;
	}
	return false;
}

static Common::String bytesToLatin1(const Common::Array<byte> &bytes) {
	Common::String out;
	out.reserve(bytes.size());
	for (uint32 i = 0; i < bytes.size(); ++i)
		out += (char)bytes[i];
	return out;
}

} // namespace

bool ActorSequenceTableLoader::loadPacked(
		const SharedActorAssets &assets,
		ActorPackedResourceHost &host,
		SequenceTableDocument &out,
		Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (assets.sequence.empty()) {
		errorMessage = "Actor has no SequenceTable resource path";
		return false;
	}

	Common::Array<byte> packed;
	if (!host.readBinary(assets.sequence, packed)) {
		errorMessage = Common::String::format("Unable to read %s",
			assets.sequence.toString().c_str());
		return false;
	}

	Common::Array<byte> decoded;
	ResourceDecoder decoder;
	if (!decoder.decodeJFX1(packed, decoded, errorMessage)) {
		if (errorMessage.empty()) {
			errorMessage = Common::String::format("Unable to decode %s",
				assets.sequence.toString().c_str());
		}
		return false;
	}

	SequenceTableForensicParser parser;
	if (!parser.parse(bytesToLatin1(decoded), out, errorMessage)) {
		if (errorMessage.empty()) {
			errorMessage = Common::String::format("Unable to parse %s",
				assets.sequence.toString().c_str());
		}
		return false;
	}

	return true;
}

void SequenceClipCorrelator::correlate(
		const SequenceTableDocument &document,
		const Common::Array<AnimationClip> &clips,
		SequenceClipCorrelation &out) const {
	out.clear();

	for (uint32 c = 0; c < clips.size(); ++c) {
		const Common::String &clipName = clips[c].name;
		bool referenced = false;

		for (uint32 d = 0; d < document.directives().size(); ++d) {
			const SequenceDirective &directive = document.directives()[d];
			if (!lineContainsIdentifier(directive.rawLine, clipName))
				continue;

			SequenceClipReference reference;
			reference.clipName = clipName;
			reference.lineNumber = directive.lineNumber;
			reference.directiveKind = directive.kind;
			reference.explicitTransition = directive.explicitTransition;
			reference.rawLine = directive.rawLine;
			out.references.push_back(reference);
			referenced = true;
		}

		for (uint32 o = 0; o < document.opaqueLines().size(); ++o) {
			const SequenceOpaqueLine &opaque = document.opaqueLines()[o];
			if (!lineContainsIdentifier(opaque.text, clipName))
				continue;

			SequenceClipReference reference;
			reference.clipName = clipName;
			reference.lineNumber = opaque.lineNumber;
			reference.directiveKind = kSequenceDirectiveUnknown;
			reference.rawLine = opaque.text;
			out.references.push_back(reference);
			referenced = true;
		}

		if (referenced) {
			if (!containsStringIgnoreCase(out.referencedClips, clipName))
				out.referencedClips.push_back(clipName);
		} else {
			out.unreferencedClips.push_back(clipName);
		}
	}
}

} // End of namespace ZeroComico
