/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage9/actor_model.h"

namespace ZeroComico {

void ActorModel::clear() {
	assets = SharedActorAssets();
	model.clear();
	animationDocument.clear();
	animationClips.clear();
	animationStats = ANJDecodeStats();
}

bool ActorModelLoader::load(const SharedActorAssets &assets,
                            ActorBinaryResourceHost &host,
                            ActorModel &out,
                            Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (!assets.valid()) {
		errorMessage = "Actor asset descriptor is incomplete";
		return false;
	}

	Common::Array<byte> p3dBytes;
	if (!host.readDecodedBinary(assets.model, p3dBytes)) {
		errorMessage = Common::String::format("Unable to decode %s",
			assets.model.toString().c_str());
		return false;
	}

	P3DModel parsedModel;
	P3DModelParser p3dParser;
	if (!p3dParser.parse(p3dBytes, parsedModel, errorMessage))
		return false;

	Common::Array<byte> anjBytes;
	if (!host.readDecodedBinary(assets.animation, anjBytes)) {
		errorMessage = Common::String::format("Unable to decode %s",
			assets.animation.toString().c_str());
		return false;
	}

	ANJDocument parsedAnimation;
	ANJDocumentParser anjParser;
	if (!anjParser.parse(anjBytes, parsedAnimation, errorMessage))
		return false;

	Common::Array<AnimationClip> clips;
	ANJDecodeStats stats;
	ANJTrackDecoder trackDecoder;
	if (!trackDecoder.decode(parsedModel, parsedAnimation, clips, stats, errorMessage))
		return false;

	ActorModel prepared;
	prepared.assets = assets;
	prepared.model = parsedModel;
	prepared.animationDocument = parsedAnimation;
	prepared.animationClips = clips;
	prepared.animationStats = stats;

	out = prepared;
	return true;
}

void registerActorSceneObjects(const ActorModel &actor, SceneRegistry &registry) {
	for (uint32 i = 0; i < actor.model.meshes.size(); ++i)
		registry.addObject(actor.model.meshes[i].name, kSceneObjectMesh);

	for (uint32 i = 0; i < actor.model.cameras.size(); ++i)
		registry.addObject(actor.model.cameras[i].name, kSceneObjectCamera);

	for (uint32 i = 0; i < actor.model.lights.size(); ++i)
		registry.addObject(actor.model.lights[i].name, kSceneObjectLight);

	// ANJ binding names may refer to transform/deformer nodes that do not own
	// a static render mesh. Preserve them in the registry as generic targets.
	for (uint32 i = 0; i < actor.animationDocument.bindings.size(); ++i) {
		const ANJBinding &binding = actor.animationDocument.bindings[i];
		if (!binding.objectName.empty())
			registry.addObject(binding.objectName, kSceneObjectOther);
	}
}

} // End of namespace ZeroComico
