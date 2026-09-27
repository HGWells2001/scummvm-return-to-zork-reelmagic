/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cstring>

#include "common/endian.h"

#include "zerocomico-stage9/anj_document.h"

namespace ZeroComico {

namespace {

static const uint16 kRecordMark = 0xAABB;
static const uint16 kGroupType = 0xF044;
static const uint16 kTimelineType = 0xF007;
static const uint16 kFileMagic = 0x0E3D;

static bool inBounds(const Common::Array<byte> &data, uint32 offset, uint32 size) {
	return offset <= data.size() && size <= data.size() - offset;
}

static uint16 readU16(const Common::Array<byte> &data, uint32 offset) {
	return READ_LE_UINT16(data.data() + offset);
}

static uint32 readU32(const Common::Array<byte> &data, uint32 offset) {
	return READ_LE_UINT32(data.data() + offset);
}

static bool hasBytes(const Common::Array<byte> &data, uint32 offset,
                     const byte *bytes, uint32 size) {
	return inBounds(data, offset, size) &&
	       memcmp(data.data() + offset, bytes, size) == 0;
}

static Common::String readName32(const Common::Array<byte> &data, uint32 offset) {
	if (!inBounds(data, offset, 32))
		return Common::String();

	uint32 length = 0;
	while (length < 32 && data[offset + length] != 0)
		++length;
	return Common::String((const char *)data.data() + offset, length);
}

static int32 findEndMark(const Common::Array<byte> &data, uint32 start, uint32 hi) {
	static const byte kEnd[3] = {0xED, 0xFF, 0xFF};
	if (hi > data.size())
		hi = data.size();

	for (uint32 p = start; p + sizeof(kEnd) <= hi; ++p) {
		if (memcmp(data.data() + p, kEnd, sizeof(kEnd)) == 0)
			return (int32)p;
	}
	return -1;
}

static bool specialNamedMark(const Common::Array<byte> &data, uint32 offset) {
	static const byte kV2[4] = {0x02, 0x00, 0x3D, 0x0E};
	static const byte kV1[4] = {0x01, 0x00, 0x3D, 0x0E};
	return hasBytes(data, offset, kV2, sizeof(kV2)) ||
	       hasBytes(data, offset, kV1, sizeof(kV1));
}

static bool walkRange(const Common::Array<byte> &data,
                      uint32 pos, uint32 hi,
                      uint16 depth, int32 parentGroup,
                      Common::Array<ANJRecord> &records) {
	if (pos == hi)
		return true;
	if (pos > hi || !inBounds(data, pos, 4))
		return false;

	const uint16 mark = readU16(data, pos);
	const uint16 type = readU16(data, pos + 2);
	const bool ordinaryMark = mark == kRecordMark;
	const bool specialMark = specialNamedMark(data, pos);
	if (!ordinaryMark && !specialMark)
		return false;

	if (ordinaryMark && type == kGroupType) {
		if (!inBounds(data, pos, 8))
			return false;

		const uint32 length = readU32(data, pos + 4);
		if (length < 4)
			return false;
		const uint32 end = pos + 4 + length;
		if (end < pos || end > hi)
			return false;

		const uint32 keep = records.size();
		ANJRecord group;
		group.offset = pos;
		group.type = type;
		group.bodyOffset = pos + 8;
		group.bodySize = end - group.bodyOffset;
		group.parentGroup = parentGroup;
		group.depth = depth;
		group.group = true;
		const int32 groupIndex = (int32)records.size();
		records.push_back(group);

		if (!walkRange(data, pos + 8, end, depth + 1, groupIndex, records) ||
		    !walkRange(data, end, hi, depth, parentGroup, records)) {
			records.resize(keep);
			return false;
		}
		return true;
	}

	if (!inBounds(data, pos + 4, 32))
		return false;

	const Common::String name = readName32(data, pos + 4);
	const uint32 bodyStart = pos + 36;
	int32 candidate = findEndMark(data, bodyStart, hi);

	while (candidate >= 0) {
		const uint32 end = (uint32)candidate;
		const uint32 keep = records.size();

		ANJRecord record;
		record.offset = pos;
		record.type = type;
		record.name = name;
		record.bodyOffset = bodyStart;
		record.bodySize = end - bodyStart;
		record.parentGroup = parentGroup;
		record.depth = depth;
		record.group = false;
		records.push_back(record);

		if (walkRange(data, end + 3, hi, depth, parentGroup, records))
			return true;

		records.resize(keep);
		candidate = findEndMark(data, end + 1, hi);
	}

	return false;
}

static void extractSemanticHeaders(ANJDocument &document) {
	for (uint32 i = 0; i < document.records.size(); ++i) {
		const ANJRecord &record = document.records[i];
		if (record.group)
			continue;

		if (record.type == kTimelineType && record.bodySize >= 48) {
			const uint32 p = record.bodyOffset;
			const uint32 marker = readU32(document.decoded, p + 36);
			if (marker != 0xF0F01234)
				continue;

			ANJTimelineHeader timeline;
			timeline.recordName = record.name;
			timeline.animationName = readName32(document.decoded, p);
			timeline.duration = readU32(document.decoded, p + 32);
			timeline.firstFrame = readU32(document.decoded, p + 40);
			timeline.lastFrame = readU32(document.decoded, p + 44);
			timeline.payloadOffset = p + 48;
			timeline.payloadSize = record.bodySize - 48;
			timeline.recordIndex = (int32)i;
			document.timelines.push_back(timeline);
			continue;
		}

		// The animation binding body recovered from the original loader is
		// exactly 68 bytes: flags + object name + parent name. It is emitted
		// under several target record types, so identify it by shape rather
		// than overfitting one type number.
		if (record.bodySize == 68) {
			const uint32 p = record.bodyOffset;
			ANJBinding binding;
			binding.type = record.type;
			binding.recordName = record.name;
			binding.flags = readU32(document.decoded, p);
			binding.objectName = readName32(document.decoded, p + 4);
			binding.parentName = readName32(document.decoded, p + 36);
			binding.recordIndex = (int32)i;
			document.bindings.push_back(binding);
		}
	}
}

} // namespace

void ANJDocument::clear() {
	version = 0;
	decoded.clear();
	records.clear();
	bindings.clear();
	timelines.clear();
}

bool ANJDocumentParser::parse(const Common::Array<byte> &decoded,
                              ANJDocument &out,
                              Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decoded.size() < 8) {
		errorMessage = "ANJ is too small";
		return false;
	}

	const uint16 version = readU16(decoded, 0);
	const uint16 magic = readU16(decoded, 2);
	if ((version != 1 && version != 2) || magic != kFileMagic) {
		errorMessage = "Invalid ANJ header";
		return false;
	}

	static const byte kTrailer[4] = {0x00, 0xED, 0xFF, 0xFF};
	if (!hasBytes(decoded, decoded.size() - 4, kTrailer, sizeof(kTrailer))) {
		errorMessage = "Invalid ANJ trailer";
		return false;
	}

	out.version = version;
	out.decoded = decoded;

	if (!walkRange(out.decoded, 4, out.decoded.size() - 4, 0, -1, out.records)) {
		errorMessage = "ANJ record graph does not tile the decoded stream";
		out.clear();
		return false;
	}

	extractSemanticHeaders(out);
	return true;
}

} // End of namespace ZeroComico
