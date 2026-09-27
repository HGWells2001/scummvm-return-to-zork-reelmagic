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

class ScriptRuntimeHost {
public:
	virtual ~ScriptRuntimeHost() {}

	virtual void setObjectVisible(const Common::String &objectName, bool visible) = 0;
	virtual bool playAnimation(const Common::String &animationName, bool loop) = 0;
	virtual bool isAnimationPlaying() const = 0;
	virtual bool isAnimationPlaying(const Common::String &animationName) const = 0;
	virtual void stopAnimation(const Common::String &animationName) = 0;
	virtual void setFocus(const Common::String &cameraName) = 0;
	virtual void requestMainPlace(const Common::String &mainPlaceName) = 0;
};

class ScriptBridge {
public:
	ScriptBridge();

	void setInput(MenuInput *input) { _input = input; }
	void setRuntimeHost(ScriptRuntimeHost *host) { _host = host; }

	bool ifObjSelected(const Common::String &objectName) const;
	bool ifObjSelected(const Common::String &ownerName,
	                   const Common::String &objectName) const;
	bool ifObjHovered(const Common::String &objectName) const;
	void clearSelectedObject();

	void e3dHide(const Common::String &objectName);
	void e3dUnhide(const Common::String &objectName);
	void setFocus(const Common::String &cameraName);

	// Native Lucifer cut-scene verbs used by the retail executable.
	bool playCut(const Common::String &cutName);
	bool playOpenCut(const Common::String &cutName);
	bool loopCut(const Common::String &cutName);
	bool waitCut(const Common::String &cutName) const;
	void stopCut(const Common::String &cutName);

	// Compatibility helpers retained for Stage 5 integration.
	bool playAnimation(const Common::String &animationName, bool loop = false);
	bool waitAnimation() const;

	void changeMainPlace(const Common::String &mainPlaceName);

private:
	MenuInput *_input;
	ScriptRuntimeHost *_host;
};

} // End of namespace ZeroComico

#endif
