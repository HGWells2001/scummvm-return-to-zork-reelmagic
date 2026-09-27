/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE6_MENU_MATERIAL_STATE_H
#define ZEROCOMICO_STAGE6_MENU_MATERIAL_STATE_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/menu_controller.h"

namespace ZeroComico {

struct MenuMaterialBinding {
	Common::String objectName;
	Common::String baseTexture;
	Common::String overrideTexture;
};

class MenuMaterialState : public MenuVisualHost {
public:
	void clear();

	void bindObjectTexture(const Common::String &objectName,
	                       const Common::String &textureName);

	Common::String objectTexture(const Common::String &objectName) const override;
	bool setObjectTextureOverride(const Common::String &objectName,
	                              const Common::String &textureName) override;
	void clearObjectTextureOverride(const Common::String &objectName) override;

	Common::String effectiveTexture(const Common::String &objectName) const;

private:
	int find(const Common::String &objectName) const;

	Common::Array<MenuMaterialBinding> _bindings;
};

} // End of namespace ZeroComico

#endif
