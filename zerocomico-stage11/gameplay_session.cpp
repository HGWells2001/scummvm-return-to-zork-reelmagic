/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage11/gameplay_session.h"

namespace ZeroComico {

GameplaySession::GameplaySession() :
	_host(nullptr) {
}

void GameplaySession::clear() {
	_gameplay.clear();
	_host = nullptr;
}

bool GameplaySession::prepare(GameplayMainPlaceState *state,
                              const Common::String &decodedRoomScript,
                              const Common::String &decodedPuzzleScript,
                              const Common::String &decodedDialogScript,
                              IntegratedGameplayHost *host,
                              ScriptBridge *bridge,
                              Common::String &errorMessage) {
	clear();
	errorMessage.clear();
	if (!state || !host || !bridge) {
		errorMessage = "GameplaySession requires state, host and ScriptBridge";
		return false;
	}

	if (!host->loadDialogue(decodedDialogScript, errorMessage))
		return false;

	if (!_gameplay.prepare(state, decodedRoomScript, decodedPuzzleScript,
	                       host, bridge, errorMessage)) {
		if (errorMessage.empty() && !host->lastError().empty())
			errorMessage = host->lastError();
		return false;
	}

	_host = host;
	return true;
}

void GameplaySession::update(uint32 deltaMillis) {
	_gameplay.update(deltaMillis);
	if (_host)
		_host->update(deltaMillis);
}

} // End of namespace ZeroComico
