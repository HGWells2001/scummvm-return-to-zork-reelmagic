/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_MAINPLACE_H
#define ZEROCOMICO_STAGE6_MAINPLACE_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

namespace ZeroComico {

struct MainPlaceDescriptor {
	Common::String name;
	Common::String startPlace;
	Common::Array<Common::String> rooms;

	bool valid() const {
		return !name.empty() && !startPlace.empty();
	}
};

/**
 * Parse the structural declarations needed for a safe MainPlace transition.
 *
 * Input is the already JFX1/LZHUF-decoded gameplay/room.isc text.
 */
bool parseMainPlaceDescriptor(const Common::String &decodedRoomScript,
                              MainPlaceDescriptor &out);

Common::String normalizeMainPlaceName(const Common::String &name);
Common::Path mainPlaceRoomScriptPath(const Common::String &name);
Common::Path mainPlaceSceneScriptPath(const Common::String &name);

} // End of namespace ZeroComico

#endif
