/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_RESOURCE_DECODER_H
#define ZEROCOMICO_STAGE9_RESOURCE_DECODER_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace ZeroComico {

struct JGF5Image {
	uint32 width;
	uint32 height;
	Common::Array<byte> bgra;

	JGF5Image() : width(0), height(0) {}
	void clear() {
		width = 0;
		height = 0;
		bgra.clear();
	}
};

class ResourceDecoder {
public:
	bool decodeJFX1(const Common::Array<byte> &packed,
	                Common::Array<byte> &decoded,
	                Common::String &errorMessage) const;

	bool decodeJGF5(const Common::Array<byte> &packed,
	                JGF5Image &image,
	                Common::String &errorMessage) const;

private:
	bool decodeLZHUF(const byte *encoded, uint32 encodedSize,
	                 uint32 decodedSize,
	                 Common::Array<byte> &decoded,
	                 Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
