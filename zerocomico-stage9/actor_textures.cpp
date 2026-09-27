/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage9/actor_textures.h"

namespace ZeroComico {

Common::Path actorRelativeResourcePath(const SharedActorAssets &assets,
                                       const Common::String &resourceName) {
	if (resourceName.empty())
		return Common::Path();

	return assets.model.getParent().append(resourceName);
}

void ActorTextureSet::clear() {
	_textures.clear();
	_missing.clear();
}

const ActorTexture *ActorTextureSet::forMaterial(int32 materialIndex) const {
	for (uint32 i = 0; i < _textures.size(); ++i) {
		if (_textures[i].materialIndex == materialIndex)
			return &_textures[i];
	}
	return nullptr;
}

bool ActorTextureSet::load(const ActorModel &actor,
                           ActorPackedResourceHost &host,
                           Common::String &errorMessage) {
	clear();
	errorMessage.clear();

	ResourceDecoder decoder;

	for (uint32 i = 0; i < actor.model.materials.size(); ++i) {
		const P3DMaterial &material = actor.model.materials[i];
		if (material.textureResource.empty())
			continue;

		ActorTexture texture;
		texture.materialIndex = (int32)i;
		texture.resourceName = material.textureResource;
		texture.resolvedPath =
			actorRelativeResourcePath(actor.assets, material.textureResource);

		Common::Array<byte> packed;
		if (!host.readBinary(texture.resolvedPath, packed)) {
			_missing.push_back(texture.resolvedPath);
			continue;
		}

		if (!decoder.decodeJGF5(packed, texture.image, errorMessage)) {
			errorMessage = Common::String::format(
				"Unable to decode actor texture %s: %s",
				texture.resolvedPath.toString().c_str(),
				errorMessage.c_str());
			return false;
		}

		_textures.push_back(texture);
	}

	// Missing texture resources are not a fatal parse error. The retail disc
	// itself contains a small number of model references with no matching
	// image, and the original engine has an explicit missing-image path.
	return true;
}

} // End of namespace ZeroComico
