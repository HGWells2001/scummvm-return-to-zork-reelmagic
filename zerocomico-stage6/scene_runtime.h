/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_SCENE_RUNTIME_H
#define ZEROCOMICO_STAGE6_SCENE_RUNTIME_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage6/script_bridge.h"

namespace ZeroComico {

enum SceneObjectType {
	kSceneObjectMesh,
	kSceneObjectCamera,
	kSceneObjectLight,
	kSceneObjectOther
};

struct SceneObjectState {
	Common::String name;
	SceneObjectType type;
	TransformSample baseTransform;
	TransformSample currentTransform;
	bool scriptVisible;

	SceneObjectState() :
		type(kSceneObjectOther),
		scriptVisible(true) {
	}

	bool visible() const {
		return scriptVisible && currentTransform.visible;
	}
};

/**
 * Stable name/id registry shared by the renderer, picker and script VM.
 */
class SceneRegistry {
public:
	void clear();

	int32 addObject(const Common::String &name, SceneObjectType type,
	                const TransformSample &base = TransformSample());

	int32 findObject(const Common::String &name) const;
	SceneObjectState *object(int32 id);
	const SceneObjectState *object(int32 id) const;

	void setVisible(const Common::String &name, bool visible);

	uint32 size() const { return _objects.size(); }
	const Common::Array<Common::String> &names() const { return _names; }

private:
	Common::Array<SceneObjectState> _objects;
	Common::Array<Common::String> _names;
};

/**
 * Concrete ScriptRuntimeHost for Stage 6.
 *
 * The Stage 4 ANJ decoder fills AnimationClip objects; the Stage 5 renderer
 * reads current transforms and visibility from SceneRegistry.
 */
class SceneRuntime : public ScriptRuntimeHost {
public:
	SceneRuntime();

	SceneRegistry &registry() { return _registry; }
	const SceneRegistry &registry() const { return _registry; }

	void setAnimationClips(const Common::Array<AnimationClip> *clips);
	void update(uint32 deltaMillis);

	const Common::String &focusCamera() const { return _focusCamera; }

	bool hasPendingMainPlace() const { return !_pendingMainPlace.empty(); }
	Common::String consumePendingMainPlace();

	// ScriptRuntimeHost
	void setObjectVisible(const Common::String &objectName, bool visible) override;
	bool playAnimation(const Common::String &animationName, bool loop) override;
	bool isAnimationPlaying() const override;
	void setFocus(const Common::String &cameraName) override;
	void requestMainPlace(const Common::String &mainPlaceName) override;

private:
	const AnimationClip *findClip(const Common::String &name) const;
	void applyAnimation();

	SceneRegistry _registry;
	const Common::Array<AnimationClip> *_clips;
	AnimationPlayer _player;
	Common::String _focusCamera;
	Common::String _pendingMainPlace;
};

} // End of namespace ZeroComico

#endif
