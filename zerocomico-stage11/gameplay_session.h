/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE11_GAMEPLAY_SESSION_H
#define ZEROCOMICO_STAGE11_GAMEPLAY_SESSION_H

#include "common/str.h"

#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage8/gameplay_runtime.h"
#include "zerocomico-stage11/integrated_gameplay_host.h"

namespace ZeroComico {

/**
 * Keeps update ordering explicit:
 *  1. gameplay/path/VM advances and can change the walking state;
 *  2. actor ANJ animation samples that resulting state.
 */
class GameplaySession {
public:
	GameplaySession();

	bool prepare(GameplayMainPlaceState *state,
	             const Common::String &decodedRoomScript,
	             const Common::String &decodedPuzzleScript,
	             const Common::String &decodedDialogScript,
	             IntegratedGameplayHost *host,
	             ScriptBridge *bridge,
	             Common::String &errorMessage);

	void clear();
	void update(uint32 deltaMillis);

	GameplayRuntime &gameplay() { return _gameplay; }
	const GameplayRuntime &gameplay() const { return _gameplay; }

private:
	GameplayRuntime _gameplay;
	IntegratedGameplayHost *_host;
};

} // End of namespace ZeroComico

#endif
