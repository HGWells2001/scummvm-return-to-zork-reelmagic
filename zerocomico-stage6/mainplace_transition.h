/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE6_MAINPLACE_TRANSITION_H
#define ZEROCOMICO_STAGE6_MAINPLACE_TRANSITION_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

class SceneRuntime;

class MainPlaceTransitionHost {
public:
	virtual ~MainPlaceTransitionHost() {}

	/**
	 * Read and decode a JFX1/LZHUF text resource.
	 */
	virtual bool readDecodedText(const Common::Path &path, Common::String &text) = 0;

	/**
	 * Replace the active MainPlace only after the target descriptor has been
	 * parsed successfully. Implementations should prepare target resources
	 * first, then swap scene state at a safe frame boundary.
	 */
	virtual bool activateMainPlace(const Common::String &directoryName,
	                               const MainPlaceDescriptor &descriptor,
	                               Common::String &errorMessage) = 0;
};

enum MainPlaceTransitionResult {
	kMainPlaceNoTransition,
	kMainPlaceTransitionDone,
	kMainPlaceTransitionReadError,
	kMainPlaceTransitionParseError,
	kMainPlaceTransitionActivateError
};

class MainPlaceTransitionController {
public:
	MainPlaceTransitionController();

	MainPlaceTransitionResult process(SceneRuntime &runtime,
	                                  MainPlaceTransitionHost &host);

	const Common::String &lastError() const { return _lastError; }
	const MainPlaceDescriptor &preparedDescriptor() const { return _prepared; }

private:
	Common::String _lastError;
	MainPlaceDescriptor _prepared;
};

} // End of namespace ZeroComico

#endif
