/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage6/menu_material_state.h"

namespace ZeroComico {

void MenuMaterialState::clear() {
	_bindings.clear();
}

int MenuMaterialState::find(const Common::String &objectName) const {
	for (uint32 i = 0; i < _bindings.size(); ++i) {
		if (_bindings[i].objectName.equalsIgnoreCase(objectName))
			return (int)i;
	}
	return -1;
}

void MenuMaterialState::bindObjectTexture(const Common::String &objectName,
                                          const Common::String &textureName) {
	const int i = find(objectName);
	if (i >= 0) {
		_bindings[i].baseTexture = textureName;
		_bindings[i].overrideTexture.clear();
		return;
	}

	MenuMaterialBinding binding;
	binding.objectName = objectName;
	binding.baseTexture = textureName;
	_bindings.push_back(binding);
}

Common::String MenuMaterialState::objectTexture(const Common::String &objectName) const {
	const int i = find(objectName);
	if (i < 0)
		return Common::String();

	if (!_bindings[i].overrideTexture.empty())
		return _bindings[i].overrideTexture;
	return _bindings[i].baseTexture;
}

bool MenuMaterialState::setObjectTextureOverride(const Common::String &objectName,
                                                 const Common::String &textureName) {
	const int i = find(objectName);
	if (i < 0 || textureName.empty())
		return false;

	_bindings[i].overrideTexture = textureName;
	return true;
}

void MenuMaterialState::clearObjectTextureOverride(const Common::String &objectName) {
	const int i = find(objectName);
	if (i >= 0)
		_bindings[i].overrideTexture.clear();
}

Common::String MenuMaterialState::effectiveTexture(const Common::String &objectName) const {
	return objectTexture(objectName);
}

} // End of namespace ZeroComico
