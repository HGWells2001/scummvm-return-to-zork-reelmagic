/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_MENU_INPUT_H
#define ZEROCOMICO_STAGE6_MENU_INPUT_H

#include "common/array.h"
#include "common/events.h"
#include "common/point.h"
#include "common/str.h"

namespace ZeroComico {

class ScenePicker;

/**
 * Converts ScummVM mouse events into Lucifer/Zero Comico object selection.
 *
 * Mesh ids come from the scene registry.  The script VM never needs to know
 * screen rectangles; it receives the original object name selected by the
 * rendered scene.
 */
class MenuInput {
public:
	MenuInput();

	void setMeshNames(const Common::Array<Common::String> *meshNames);
	void reset();

	void handleEvent(const Common::Event &event, const ScenePicker &picker);

	const Common::String &hoveredObject() const { return _hoveredObject; }
	const Common::String &pressedObject() const { return _pressedObject; }
	const Common::String &activatedObject() const { return _activatedObject; }

	bool isHovered(const Common::String &name) const;
	bool isActivated(const Common::String &name) const;

	/**
	 * Return the last clicked object and clear the click latch.
	 */
	Common::String consumeActivatedObject();

	const Common::Point &mousePos() const { return _mousePos; }

private:
	Common::String nameForId(int32 id) const;
	void updateHover(const Common::Point &p, const ScenePicker &picker);

	const Common::Array<Common::String> *_meshNames;
	Common::Point _mousePos;
	Common::String _hoveredObject;
	Common::String _pressedObject;
	Common::String _activatedObject;
	bool _leftDown;
};

} // End of namespace ZeroComico

#endif
