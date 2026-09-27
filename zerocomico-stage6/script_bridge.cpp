/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/script_bridge.h"
#include "zerocomico-stage6/menu_input.h"

namespace ZeroComico {

ScriptBridge::ScriptBridge() : _input(nullptr), _host(nullptr) {
}

bool ScriptBridge::ifObjSelected(const Common::String &objectName) const {
	return _input && _input->isActivated(objectName);
}

bool ScriptBridge::ifObjHovered(const Common::String &objectName) const {
	return _input && _input->isHovered(objectName);
}

void ScriptBridge::clearSelectedObject() {
	if (_input)
		_input->consumeActivatedObject();
}

void ScriptBridge::e3dHide(const Common::String &objectName) {
	if (_host)
		_host->setObjectVisible(objectName, false);
}

void ScriptBridge::e3dUnhide(const Common::String &objectName) {
	if (_host)
		_host->setObjectVisible(objectName, true);
}

bool ScriptBridge::playAnimation(const Common::String &animationName, bool loop) {
	return _host && _host->playAnimation(animationName, loop);
}

bool ScriptBridge::waitAnimation() const {
	return _host && _host->isAnimationPlaying();
}

void ScriptBridge::setFocus(const Common::String &cameraName) {
	if (_host)
		_host->setFocus(cameraName);
}

void ScriptBridge::changeMainPlace(const Common::String &mainPlaceName) {
	if (_host)
		_host->requestMainPlace(mainPlaceName);
}

} // End of namespace ZeroComico
