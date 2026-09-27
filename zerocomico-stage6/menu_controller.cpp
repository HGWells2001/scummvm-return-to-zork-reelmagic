/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage6/menu_controller.h"

#include "zerocomico-stage6/menu_highlight.h"
#include "zerocomico-stage6/menu_input.h"
#include "zerocomico-stage6/scene_picker.h"

namespace ZeroComico {

MenuController::MenuController() :
	_input(nullptr),
	_visualHost(nullptr) {
}

void MenuController::clearHighlight() {
	if (_visualHost && !_highlightedObject.empty())
		_visualHost->clearObjectTextureOverride(_highlightedObject);
	_highlightedObject.clear();
}

void MenuController::updateHighlight() {
	if (!_input || !_visualHost)
		return;

	const Common::String hovered = _input->hoveredObject();
	if (hovered.equalsIgnoreCase(_highlightedObject))
		return;

	clearHighlight();

	if (hovered.empty())
		return;

	const Common::String currentTexture = _visualHost->objectTexture(hovered);
	const Common::String litTexture = menuButtonLitTexture(currentTexture);
	if (litTexture.empty())
		return;

	if (_visualHost->setObjectTextureOverride(hovered, litTexture))
		_highlightedObject = hovered;
}

void MenuController::handleEvent(const Common::Event &event, const ScenePicker &picker) {
	if (!_input)
		return;

	_input->handleEvent(event, picker);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
		updateHighlight();
		break;
	default:
		break;
	}
}

void MenuController::reset() {
	clearHighlight();
	if (_input)
		_input->reset();
}

} // End of namespace ZeroComico
