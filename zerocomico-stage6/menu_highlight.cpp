/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage6/menu_highlight.h"

namespace ZeroComico {

static const char *const kLitSuffix = "_acc.tga";
static const char *const kUnlitSuffix = "_spe.tga";
static const uint kSuffixLength = 8;

static Common::String lowerCopy(const Common::String &s) {
	Common::String out = s;
	out.toLowercase();
	return out;
}

bool isMenuButtonTexture(const Common::String &textureName) {
	const Common::String lower = lowerCopy(textureName);
	return lower.hasSuffix(kLitSuffix) || lower.hasSuffix(kUnlitSuffix);
}

Common::String menuButtonLitTexture(const Common::String &textureName) {
	const Common::String lower = lowerCopy(textureName);
	if (lower.hasSuffix(kLitSuffix))
		return textureName;
	if (!lower.hasSuffix(kUnlitSuffix) || textureName.size() < kSuffixLength)
		return Common::String();

	return textureName.substr(0, textureName.size() - kSuffixLength) + kLitSuffix;
}

Common::String menuButtonUnlitTexture(const Common::String &textureName) {
	const Common::String lower = lowerCopy(textureName);
	if (lower.hasSuffix(kUnlitSuffix))
		return textureName;
	if (!lower.hasSuffix(kLitSuffix) || textureName.size() < kSuffixLength)
		return Common::String();

	return textureName.substr(0, textureName.size() - kSuffixLength) + kUnlitSuffix;
}

} // End of namespace ZeroComico
