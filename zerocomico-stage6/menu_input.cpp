/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/menu_input.h"
#include "zerocomico-stage6/scene_picker.h"

namespace ZeroComico {

MenuInput::MenuInput() :
	_meshNames(nullptr),
	_mousePos(0, 0),
	_leftDown(false) {
}

void MenuInput::setMeshNames(const Common::Array<Common::String> *meshNames) {
	_meshNames = meshNames;
	reset();
}

void MenuInput::reset() {
	_hoveredObject.clear();
	_pressedObject.clear();
	_activatedObject.clear();
	_leftDown = false;
}

Common::String MenuInput::nameForId(int32 id) const {
	if (!_meshNames || id < 0 || (uint32)id >= _meshNames->size())
		return Common::String();
	return (*_meshNames)[id];
}

void MenuInput::updateHover(const Common::Point &p, const ScenePicker &picker) {
	_hoveredObject = nameForId(picker.pick(p.x, p.y));
}

void MenuInput::handleEvent(const Common::Event &event, const ScenePicker &picker) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		_mousePos = event.mouse;
		updateHover(_mousePos, picker);
		break;

	case Common::EVENT_LBUTTONDOWN:
		_mousePos = event.mouse;
		updateHover(_mousePos, picker);
		_leftDown = true;
		_pressedObject = _hoveredObject;
		break;

	case Common::EVENT_LBUTTONUP: {
		_mousePos = event.mouse;
		updateHover(_mousePos, picker);

		if (_leftDown && !_pressedObject.empty() &&
		    _pressedObject.equalsIgnoreCase(_hoveredObject)) {
			_activatedObject = _hoveredObject;
		}

		_leftDown = false;
		_pressedObject.clear();
		break;
	}

	default:
		break;
	}
}

bool MenuInput::isHovered(const Common::String &name) const {
	return !_hoveredObject.empty() && _hoveredObject.equalsIgnoreCase(name);
}

bool MenuInput::isActivated(const Common::String &name) const {
	return !_activatedObject.empty() && _activatedObject.equalsIgnoreCase(name);
}

Common::String MenuInput::consumeActivatedObject() {
	Common::String result = _activatedObject;
	_activatedObject.clear();
	return result;
}

} // End of namespace ZeroComico
