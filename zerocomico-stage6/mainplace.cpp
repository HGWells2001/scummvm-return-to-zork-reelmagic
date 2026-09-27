/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/mainplace.h"

#include "common/tokenizer.h"

namespace ZeroComico {

Common::String normalizeMainPlaceName(const Common::String &name) {
	if (name.empty())
		return Common::String();

	Common::String lower = name;
	lower.toLowercase();

	if (lower.size() >= 2 && lower[0] == 'm' && lower[1] == 'p') {
		Common::String result("Mp");
		result += lower.substr(2);
		return result;
	}

	return name;
}

Common::Path mainPlaceRoomScriptPath(const Common::String &name) {
	const Common::String normalized = normalizeMainPlaceName(name);
	return Common::Path(Common::String::format("%s/gameplay/room.isc", normalized.c_str()));
}

Common::Path mainPlaceSceneScriptPath(const Common::String &name) {
	const Common::String normalized = normalizeMainPlaceName(name);
	return Common::Path(Common::String::format("%s/gameplay/scene.isc", normalized.c_str()));
}

bool parseMainPlaceDescriptor(const Common::String &decodedRoomScript,
                              MainPlaceDescriptor &out) {
	out = MainPlaceDescriptor();

	Common::StringTokenizer tokens(decodedRoomScript);
	while (!tokens.empty()) {
		const Common::String token = tokens.nextToken();

		if (token.equalsIgnoreCase("ge_MainPlace")) {
			if (!tokens.empty())
				out.name = tokens.nextToken();
			continue;
		}

		if (token.equalsIgnoreCase("StartPlace:") ||
		    token.equalsIgnoreCase("StartPlace")) {
			if (!tokens.empty())
				out.startPlace = tokens.nextToken();
			continue;
		}

		if (token.equalsIgnoreCase("Room")) {
			if (!tokens.empty())
				out.rooms.push_back(tokens.nextToken());
			continue;
		}
	}

	out.name = normalizeMainPlaceName(out.name);
	return out.valid();
}

} // End of namespace ZeroComico
