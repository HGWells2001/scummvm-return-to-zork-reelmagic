/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE20_CHARACTER_START_PLACEMENT_H
#define ZEROCOMICO_STAGE20_CHARACTER_START_PLACEMENT_H

#include "common/str.h"

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage16/character_start_binding.h"
#include "zerocomico-stage19/shape_geometry.h"

namespace ZeroComico {

struct CharacterStartPlacement {
	CharacterStartBinding binding;
	ShapeGeometryDocument geometry;
	CharacterStartGeometry resolvedGeometry;

	void clear() {
		binding.clear();
		geometry.clear();
		resolvedGeometry = CharacterStartGeometry();
	}

	bool valid() const {
		return binding.valid() && resolvedGeometry.valid();
	}
};

/**
 * Loads the retail char.isc / Shape.shp chain and publishes both the
 * symbolic Position helper binding and its decoded A/B geometry.
 *
 * No A/B semantics are assigned here.
 */
class CharacterStartPlacementLoader {
public:
	bool load(const Common::String &mainPlaceName,
	          const Common::String &characterName,
	          GameplayResourceHost &resources,
	          CharacterStartPlacement &out,
	          Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
