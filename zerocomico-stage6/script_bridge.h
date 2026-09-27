/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_SCRIPT_BRIDGE_H
#define ZEROCOMICO_STAGE6_SCRIPT_BRIDGE_H

#include "common/str.h"

namespace ZeroComico {

class MenuInput;

/**
 * Operations the script VM can request from the 3D/runtime layer.
 *
 * Keeping this interface narrow lets the existing Stage 3 script interpreter
 * remain independent from the Stage 5 renderer.
 */
class ScriptRuntimeHost {
public:
	virtual ~ScriptRuntimeHost() {}

	virtual void setObjectVisible(const Common::String &objectName, bool visible) = 0;
	virtual bool playAnimation(const Common::String &animationName, bool loop) = 0;
	virtual bool isAnimationPlaying() const = 0;
	virtual void setFocus(const Common::String &cameraName) = 0;

	/**
	 * Request a level/MainPlace transition such as Mp0 -> Mp1.
	 * The host performs the actual resource unload/load at a safe point.
	 */
	virtual void requestMainPlace(const Common::String &mainPlaceName) = 0;
};

class ScriptBridge {
public:
	ScriptBridge();

	void setInput(MenuInput *input) { _input = input; }
	void setRuntimeHost(ScriptRuntimeHost *host) { _host = host; }

	// Input predicates used by script conditionals.
	bool ifObjSelected(const Common::String &objectName) const;

	/**
	 * Original Lucifer syntax is two-argument:
	 *     ifobjselected <owner/context> <object>
	 * The owner is retained in the API for faithful parsing even though Stage 6
	 * selection currently resolves the selected scene object globally.
	 */
	bool ifObjSelected(const Common::String &ownerName,
	                   const Common::String &objectName) const;

	bool ifObjHovered(const Common::String &objectName) const;
	void clearSelectedObject();

	// Engine3D/runtime verbs.
	void e3dHide(const Common::String &objectName);
	void e3dUnhide(const Common::String &objectName);
	bool playAnimation(const Common::String &animationName, bool loop = false);
	bool waitAnimation() const;
	void setFocus(const Common::String &cameraName);
	void changeMainPlace(const Common::String &mainPlaceName);

private:
	MenuInput *_input;
	ScriptRuntimeHost *_host;
};

} // End of namespace ZeroComico

#endif
