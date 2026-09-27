/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cstring>

#include "common/endian.h"

#include "zerocomico-stage9/resource_decoder.h"

namespace ZeroComico {

namespace {

static const uint32 kN = 4096;
static const uint32 kF = 60;
static const uint32 kThreshold = 2;
static const uint32 kNChar = 256 - kThreshold + kF; // 314
static const uint32 kT = kNChar * 2 - 1;            // 627
static const uint32 kR = kT - 1;                    // 626
static const uint32 kMaxFreq = 0x8000;

static const byte kPCode[64] = {
	0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
	0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
	0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
	0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

class BitReader {
public:
	BitReader(const byte *data, uint32 size) :
		_data(data), _size(size), _index(0), _buffer(0), _bits(0) {
	}

	uint32 bit() {
		fill();
		const uint32 value = (_buffer >> 15) & 1;
		_buffer = (uint16)(_buffer << 1);
		--_bits;
		return value;
	}

	uint32 byteValue() {
		fill();
		const uint32 value = (_buffer >> 8) & 0xFF;
		_buffer = (uint16)(_buffer << 8);
		_bits -= 8;
		return value;
	}

private:
	void fill() {
		while (_bits <= 8) {
			const uint32 value = _index < _size ? _data[_index++] : 0;
			_buffer |= (uint16)(value << (8 - _bits));
			_bits += 8;
		}
	}

	const byte *_data;
	uint32 _size;
	uint32 _index;
	uint16 _buffer;
	int32 _bits;
};

struct PositionTables {
	byte decodeCode[256];
	byte decodeLength[256];

	PositionTables() {
		memset(decodeCode, 0, sizeof(decodeCode));
		memset(decodeLength, 0, sizeof(decodeLength));

		for (uint32 i = 0; i < 64; ++i) {
			const uint32 lo = kPCode[i];
			const uint32 hi = i + 1 < 64 ? kPCode[i + 1] : 256;
			const uint32 width = hi - lo;

			uint32 bits = 0;
			uint32 w = width;
			while (w > 1) {
				w >>= 1;
				++bits;
			}
			const uint32 length = 8 - bits;

			for (uint32 b = lo; b < hi; ++b) {
				decodeCode[b] = (byte)i;
				decodeLength[b] = (byte)length;
			}
		}
	}
};

class AdaptiveHuffman {
public:
	AdaptiveHuffman() {
		for (uint32 i = 0; i < ARRAYSIZE(_freq); ++i)
			_freq[i] = 0;
		for (uint32 i = 0; i < ARRAYSIZE(_parent); ++i)
			_parent[i] = 0;
		for (uint32 i = 0; i < ARRAYSIZE(_son); ++i)
			_son[i] = 0;

		for (uint32 i = 0; i < kNChar; ++i) {
			_freq[i] = 1;
			_son[i] = i + kT;
			_parent[i + kT] = i;
		}

		uint32 i = 0;
		uint32 j = kNChar;
		while (j <= kR) {
			_freq[j] = _freq[i] + _freq[i + 1];
			_son[j] = i;
			_parent[i] = j;
			_parent[i + 1] = j;
			i += 2;
			++j;
		}

		_freq[kT] = 0xFFFF;
		_parent[kR] = 0;
	}

	uint32 decodeChar(BitReader &bits) {
		uint32 c = _son[kR];
		while (c < kT) {
			c += bits.bit();
			c = _son[c];
		}
		c -= kT;
		update(c);
		return c;
	}

private:
	void reconstruct() {
		uint32 j = 0;
		for (uint32 i = 0; i < kT; ++i) {
			if (_son[i] >= kT) {
				_freq[j] = (_freq[i] + 1) / 2;
				_son[j] = _son[i];
				++j;
			}
		}

		uint32 i = 0;
		j = kNChar;
		while (j < kT) {
			uint32 k = i + 1;
			const uint32 f = _freq[i] + _freq[k];
			_freq[j] = f;
			k = j - 1;
			while (f < _freq[k])
				--k;
			++k;

			for (uint32 move = j; move > k; --move) {
				_freq[move] = _freq[move - 1];
				_son[move] = _son[move - 1];
			}
			_freq[k] = f;
			_son[k] = i;
			i += 2;
			++j;
		}

		for (uint32 n = 0; n < kT; ++n) {
			const uint32 child = _son[n];
			if (child >= kT) {
				_parent[child] = n;
			} else {
				_parent[child] = n;
				_parent[child + 1] = n;
			}
		}
	}

	void update(uint32 symbol) {
		if (_freq[kR] == kMaxFreq)
			reconstruct();

		uint32 c = _parent[symbol + kT];
		while (true) {
			++_freq[c];
			const uint32 frequency = _freq[c];
			uint32 l = c + 1;

			if (frequency > _freq[l]) {
				while (frequency > _freq[l + 1])
					++l;

				_freq[c] = _freq[l];
				_freq[l] = frequency;

				const uint32 i = _son[c];
				_parent[i] = l;
				if (i < kT)
					_parent[i + 1] = l;

				const uint32 j = _son[l];
				_son[l] = i;
				_parent[j] = c;
				if (j < kT)
					_parent[j + 1] = c;
				_son[c] = j;
				c = l;
			}

			c = _parent[c];
			if (c == 0)
				break;
		}
	}

	uint32 _freq[kT + 1];
	uint32 _parent[kT + kNChar];
	uint32 _son[kT];
};

static uint32 decodePosition(BitReader &bits, const PositionTables &tables) {
	uint32 i = bits.byteValue();
	const uint32 c = (uint32)tables.decodeCode[i] << 6;
	int32 j = (int32)tables.decodeLength[i] - 2;

	while (j > 0) {
		i = ((i << 1) + bits.bit()) & 0xFFFF;
		--j;
	}
	return c | (i & 0x3F);
}

static bool hasMagic(const Common::Array<byte> &data, const char *magic) {
	return data.size() >= 4 && memcmp(data.data(), magic, 4) == 0;
}

} // namespace

bool ResourceDecoder::decodeLZHUF(const byte *encoded, uint32 encodedSize,
                                  uint32 decodedSize,
                                  Common::Array<byte> &decoded,
                                  Common::String &errorMessage) const {
	decoded.clear();
	errorMessage.clear();

	if (decodedSize == 0)
		return true;
	if (!encoded && encodedSize != 0) {
		errorMessage = "LZHUF stream pointer is null";
		return false;
	}

	decoded.reserve(decodedSize);

	byte ring[kN];
	memset(ring, 0x20, sizeof(ring));
	uint32 ringPos = kN - kF;

	BitReader bits(encoded, encodedSize);
	AdaptiveHuffman huffman;
	PositionTables positions;

	while (decoded.size() < decodedSize) {
		const uint32 c = huffman.decodeChar(bits);

		if (c < 256) {
			const byte value = (byte)c;
			decoded.push_back(value);
			ring[ringPos] = value;
			ringPos = (ringPos + 1) & (kN - 1);
			continue;
		}

		const uint32 matchPos =
			(ringPos - decodePosition(bits, positions) - 1) & (kN - 1);
		const uint32 matchLength = c - 255 + kThreshold;

		for (uint32 k = 0; k < matchLength && decoded.size() < decodedSize; ++k) {
			const byte value = ring[(matchPos + k) & (kN - 1)];
			decoded.push_back(value);
			ring[ringPos] = value;
			ringPos = (ringPos + 1) & (kN - 1);
		}
	}

	if (decoded.size() != decodedSize) {
		errorMessage = Common::String::format(
			"LZHUF length mismatch: got %u, expected %u",
			(uint32)decoded.size(), decodedSize);
		return false;
	}
	return true;
}

bool ResourceDecoder::decodeJFX1(const Common::Array<byte> &packed,
                                 Common::Array<byte> &decoded,
                                 Common::String &errorMessage) const {
	decoded.clear();
	errorMessage.clear();

	if (!hasMagic(packed, "JFX1") || packed.size() < 12) {
		errorMessage = "Invalid JFX1 header";
		return false;
	}

	const uint32 decodedSize = READ_LE_UINT32(packed.data() + 4);
	const uint32 encodedSize = READ_LE_UINT32(packed.data() + 8);
	if (encodedSize != packed.size() - 12) {
		errorMessage = Common::String::format(
			"JFX1 encoded-size mismatch: header %u, file %u",
			encodedSize, (uint32)packed.size() - 12);
		return false;
	}

	return decodeLZHUF(packed.data() + 12, encodedSize,
	                   decodedSize, decoded, errorMessage);
}

bool ResourceDecoder::decodeJGF5(const Common::Array<byte> &packed,
                                 JGF5Image &image,
                                 Common::String &errorMessage) const {
	image.clear();
	errorMessage.clear();

	if (!hasMagic(packed, "JGF5") || packed.size() < 28) {
		errorMessage = "Invalid JGF5 header";
		return false;
	}

	const uint32 field0 = READ_LE_UINT32(packed.data() + 4);
	const uint32 field1 = READ_LE_UINT32(packed.data() + 8);
	const uint32 width = READ_LE_UINT32(packed.data() + 12);
	const uint32 height = READ_LE_UINT32(packed.data() + 16);
	const uint32 decodedSize = READ_LE_UINT32(packed.data() + 20);
	const uint32 encodedSize = READ_LE_UINT32(packed.data() + 24);

	if (field0 != 0 || field1 != 3) {
		errorMessage = Common::String::format(
			"Unsupported JGF5 header fields %u/%u", field0, field1);
		return false;
	}
	if (encodedSize != packed.size() - 28) {
		errorMessage = "JGF5 encoded-size mismatch";
		return false;
	}
	if (width == 0 || height == 0 ||
	    width > 16384 || height > 16384 ||
	    decodedSize != width * height * 4) {
		errorMessage = "Invalid JGF5 dimensions or decoded length";
		return false;
	}

	Common::Array<byte> bgra;
	if (!decodeLZHUF(packed.data() + 28, encodedSize,
	                 decodedSize, bgra, errorMessage))
		return false;

	image.width = width;
	image.height = height;
	image.bgra = bgra;
	return true;
}

} // End of namespace ZeroComico
