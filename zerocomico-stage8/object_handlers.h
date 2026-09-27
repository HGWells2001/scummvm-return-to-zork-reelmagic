/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE8_OBJECT_HANDLERS_H
#define ZEROCOMICO_STAGE8_OBJECT_HANDLERS_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage7/gameplay_interaction.h"

namespace ZeroComico {

struct ObjectHandlerBody {
	Common::String objectName;
	GameplayVerb verb;
	Common::Array<Common::String> lines;
};

class ObjectHandlerRegistry {
public:
	void clear();
	void parse(const Common::String &decodedPuzzleScript);

	const ObjectHandlerBody *find(const Common::String &objectName,
	                              GameplayVerb verb) const;

	uint32 size() const { return _handlers.size(); }

private:
	Common::Array<ObjectHandlerBody> _handlers;
};

} // End of namespace ZeroComico

#endif
