/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE6_MENU_CONTROLLER_H
#define ZEROCOMICO_STAGE6_MENU_CONTROLLER_H

#include "common/events.h"
#include "common/str.h"

namespace ZeroComico {

class MenuInput;
class ScenePicker;

/**
 * Minimal material API needed to reproduce the retail menu's lit/unlit hover.
 */
class MenuVisualHost {
public:
	virtual ~MenuVisualHost() {}

	virtual Common::String objectTexture(const Common::String &objectName) const = 0;
	virtual bool setObjectTextureOverride(const Common::String &objectName,
	                                      const Common::String &textureName) = 0;
	virtual void clearObjectTextureOverride(const Common::String &objectName) = 0;
};

class MenuController {
public:
	MenuController();

	void setInput(MenuInput *input) { _input = input; }
	void setVisualHost(MenuVisualHost *host) { _visualHost = host; }

	/**
	 * Feed a ScummVM event after the current scene/pick buffer has been rendered.
	 */
	void handleEvent(const Common::Event &event, const ScenePicker &picker);

	void reset();

private:
	void updateHighlight();
	void clearHighlight();

	MenuInput *_input;
	MenuVisualHost *_visualHost;
	Common::String _highlightedObject;
};

} // End of namespace ZeroComico

#endif
