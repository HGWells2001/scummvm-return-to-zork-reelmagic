/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE11_INTEGRATED_GAMEPLAY_HOST_H
#define ZEROCOMICO_STAGE11_INTEGRATED_GAMEPLAY_HOST_H

#include "common/str.h"

#include "zerocomico-stage6/scene_runtime.h"
#include "zerocomico-stage8/gameplay_runtime.h"
#include "zerocomico-stage9/actor_animation_runtime.h"
#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage9/actor_render_catalog.h"
#include "zerocomico-stage9/actor_textures.h"
#include "zerocomico-stage10/gameplay_dialogue_service.h"
#include "zerocomico-stage11/actor_motion_controller.h"

namespace ZeroComico {

/**
 * Renderer/world-specific hooks that remain outside the data/runtime layer.
 *
 * In particular, converting a 2D BSP floor coordinate into the P3D actor
 * world transform is kept here until the retail axis mapping is proven.
 */
class GameplaySceneAdapter {
public:
	virtual ~GameplaySceneAdapter() {}

	virtual bool resolveEntityFloorPosition(const Common::String &entityName,
	                                        NavVec2 &position) const = 0;
	virtual void setActorFloorPosition(const Common::String &actorName,
	                                   const NavVec2 &position) = 0;
	virtual void showExamineText(const Common::String &text) = 0;
};

/**
 * Concrete Stage 8 GameplayRuntimeHost assembled from the Stage 9 actor
 * runtime and Stage 10 dialogue runtime.
 */
class IntegratedGameplayHost : public GameplayRuntimeHost {
public:
	IntegratedGameplayHost(ActorPackedResourceHost &resources,
	                       SceneRegistry &sceneRegistry,
	                       GameplaySceneAdapter &scene,
	                       DialoguePresentationHost &dialoguePresentation);

	bool loadDialogue(const Common::String &decodedDialogScript,
	                  Common::String &errorMessage);

	bool configureMotion(const ActorMotionClips &clips,
	                     Common::String &errorMessage);

	void update(uint32 deltaMillis);

	const Common::String &lastError() const { return _lastError; }
	const ActorModel &actor() const { return _actor; }
	const ActorTextureSet &actorTextures() const { return _textures; }
	const ActorRenderCatalog &renderCatalog() const { return _renderCatalog; }
	ActorAnimationRuntime &actorAnimation() { return _actorAnimation; }
	ActorMotionController &motion() { return _motion; }
	GameplayDialogueService &dialogue() { return _dialogue; }
	const GameplayDialogueService &dialogue() const { return _dialogue; }

	// GameplayRuntimeHost
	bool loadSharedActor(const SharedActorAssets &assets) override;
	bool resolveEntityFloorPosition(const Common::String &entityName,
	                                NavVec2 &position) const override;
	void setActorFloorPosition(const Common::String &actorName,
	                           const NavVec2 &position) override;
	void setActorWalking(const Common::String &actorName,
	                     bool walking) override;
	bool startDialog(const Common::String &speaker,
	                 const Common::String &dialogName) override;
	bool isDialogPlaying() const override;
	void showExamineText(const Common::String &text) override;

private:
	void clearActor();

	ActorPackedResourceHost &_resources;
	SceneRegistry &_sceneRegistry;
	GameplaySceneAdapter &_scene;
	DialoguePresentationHost &_dialoguePresentation;

	ActorModel _actor;
	ActorTextureSet _textures;
	ActorRenderCatalog _renderCatalog;
	ActorAnimationRuntime _actorAnimation;
	ActorMotionController _motion;
	GameplayDialogueService _dialogue;
	Common::String _lastError;
	bool _dialogueLoaded;
};

} // End of namespace ZeroComico

#endif
