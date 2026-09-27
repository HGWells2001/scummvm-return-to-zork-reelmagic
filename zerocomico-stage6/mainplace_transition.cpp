/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage6/mainplace_transition.h"
#include "zerocomico-stage6/scene_runtime.h"

namespace ZeroComico {

MainPlaceTransitionController::MainPlaceTransitionController() {
}

MainPlaceTransitionResult MainPlaceTransitionController::process(
		SceneRuntime &runtime, MainPlaceTransitionHost &host) {
	_lastError.clear();
	_prepared = MainPlaceDescriptor();

	if (!runtime.hasPendingMainPlace())
		return kMainPlaceNoTransition;

	const Common::String target = runtime.pendingMainPlace();
	const Common::Path roomScript = mainPlaceRoomScriptPath(target);

	Common::String decodedRoom;
	if (!host.readDecodedText(roomScript, decodedRoom)) {
		_lastError = Common::String::format("Unable to read %s",
		                                   roomScript.toString().c_str());
		return kMainPlaceTransitionReadError;
	}

	if (!parseMainPlaceDescriptor(decodedRoom, _prepared)) {
		_lastError = Common::String::format("Invalid MainPlace declaration in %s",
		                                   roomScript.toString().c_str());
		return kMainPlaceTransitionParseError;
	}

	// Zero Comico's Mp0..Mp5 room files declare matching ge_MainPlace names.
	// Rejecting a mismatch protects against stale/wrong-directory resources.
	if (!_prepared.name.equalsIgnoreCase(target)) {
		_lastError = Common::String::format(
			"MainPlace mismatch: requested %s, script declares %s",
			target.c_str(), _prepared.name.c_str());
		return kMainPlaceTransitionParseError;
	}

	Common::String activationError;
	if (!host.activateMainPlace(target, _prepared, activationError)) {
		_lastError = activationError.empty()
			? Common::String::format("Unable to activate MainPlace %s", target.c_str())
			: activationError;
		return kMainPlaceTransitionActivateError;
	}

	// Clear only after the replacement succeeded. Failed transitions stay
	// pending so the caller can report/retry without losing the requested target.
	runtime.clearPendingMainPlace();
	return kMainPlaceTransitionDone;
}

} // End of namespace ZeroComico
