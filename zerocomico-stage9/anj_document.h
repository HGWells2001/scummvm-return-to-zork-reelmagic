/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ANJ_DOCUMENT_H
#define ZEROCOMICO_STAGE9_ANJ_DOCUMENT_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace ZeroComico {

struct ANJRecord {
	uint32 offset;
	uint16 type;
	Common::String name;
	uint32 bodyOffset;
	uint32 bodySize;
	int32 parentGroup;
	uint16 depth;
	bool group;

	ANJRecord() :
		offset(0), type(0), bodyOffset(0), bodySize(0),
		parentGroup(-1), depth(0), group(false) {}
};

struct ANJBinding {
	uint16 type;
	Common::String recordName;
	Common::String objectName;
	Common::String parentName;
	uint32 flags;
	int32 recordIndex;

	ANJBinding() : type(0), flags(0), recordIndex(-1) {}
};

struct ANJTimelineHeader {
	Common::String recordName;
	Common::String animationName;
	uint32 duration;
	uint32 firstFrame;
	uint32 lastFrame;
	uint32 payloadOffset;
	uint32 payloadSize;
	int32 recordIndex;

	ANJTimelineHeader() :
		duration(0), firstFrame(0), lastFrame(0),
		payloadOffset(0), payloadSize(0), recordIndex(-1) {}
};

struct ANJDocument {
	uint16 version;
	Common::Array<byte> decoded;
	Common::Array<ANJRecord> records;
	Common::Array<ANJBinding> bindings;
	Common::Array<ANJTimelineHeader> timelines;

	void clear();
};

class ANJDocumentParser {
public:
	bool parse(const Common::Array<byte> &decoded,
	           ANJDocument &out,
	           Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
