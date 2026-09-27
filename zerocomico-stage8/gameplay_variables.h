/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE8_GAMEPLAY_VARIABLES_H
#define ZEROCOMICO_STAGE8_GAMEPLAY_VARIABLES_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

struct GameplayVariable {
	Common::String name;
	Common::String value;
};

class GameplayVariables {
public:
	void clear();

	void parseDeclarations(const Common::String &decodedScript);

	bool has(const Common::String &name) const;
	Common::String get(const Common::String &name,
	                   const Common::String &fallback = Common::String()) const;
	void set(const Common::String &name, const Common::String &value);
	bool equals(const Common::String &name, const Common::String &value) const;

	uint32 size() const { return _variables.size(); }

private:
	int32 find(const Common::String &name) const;
	Common::Array<GameplayVariable> _variables;
};

} // End of namespace ZeroComico

#endif
