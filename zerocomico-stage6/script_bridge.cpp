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

bool ScriptBridge::ifObjSelected(const Common::String &ownerName,
                                 const Common::String &objectName) const {
	(void)ownerName;
	return ifObjSelected(objectName);
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

void ScriptBridge::setFocus(const Common::String &cameraName) {
	if (_host)
		_host->setFocus(cameraName);
}

bool ScriptBridge::playCut(const Common::String &cutName) {
	return _host && _host->playAnimation(cutName, false);
}

bool ScriptBridge::playOpenCut(const Common::String &cutName) {
	// For the menu path this uses the same decoded ANJ timeline as play_cut.
	// Any original open-cut preload/display distinction can be added here
	// without changing the VM opcode adapter.
	return playCut(cutName);
}

bool ScriptBridge::loopCut(const Common::String &cutName) {
	return _host && _host->playAnimation(cutName, true);
}

bool ScriptBridge::waitCut(const Common::String &cutName) const {
	return _host && _host->isAnimationPlaying(cutName);
}

void ScriptBridge::stopCut(const Common::String &cutName) {
	if (_host)
		_host->stopAnimation(cutName);
}

bool ScriptBridge::playAnimation(const Common::String &animationName, bool loop) {
	return _host && _host->playAnimation(animationName, loop);
}

bool ScriptBridge::waitAnimation() const {
	return _host && _host->isAnimationPlaying();
}

void ScriptBridge::changeMainPlace(const Common::String &mainPlaceName) {
	if (_host)
		_host->requestMainPlace(mainPlaceName);
}

} // End of namespace ZeroComico
