/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage11/integrated_gameplay_host.h"

namespace ZeroComico {

IntegratedGameplayHost::IntegratedGameplayHost(
		ActorPackedResourceHost &resources,
		SceneRegistry &sceneRegistry,
		GameplaySceneAdapter &scene,
		DialoguePresentationHost &dialoguePresentation) :
	_resources(resources),
	_sceneRegistry(sceneRegistry),
	_scene(scene),
	_dialoguePresentation(dialoguePresentation),
	_dialogueLoaded(false) {
}

void IntegratedGameplayHost::clearActor() {
	_motion.clear();
	_actorAnimation.clear();
	_textures.clear();
	_actor.clear();
	_renderCatalog = ActorRenderCatalog();
}

bool IntegratedGameplayHost::loadSharedActor(const SharedActorAssets &assets) {
	_lastError.clear();
	clearActor();

	ActorModel prepared;
	ActorModelLoader loader;
	if (!loader.loadPacked(assets, _resources, prepared, _lastError))
		return false;

	ActorTextureSet textures;
	if (!textures.load(prepared, _resources, _lastError))
		return false;

	ActorRenderCatalog catalog;
	if (!catalog.build(prepared, _lastError))
		return false;

	_actor = prepared;
	_textures = textures;
	_renderCatalog = catalog;

	registerActorSceneObjects(_actor, _sceneRegistry);
	_actorAnimation.bind(&_actor, &_sceneRegistry);
	_motion.bind(&_actorAnimation);
	return true;
}

bool IntegratedGameplayHost::loadDialogue(
		const Common::String &decodedDialogScript,
		Common::String &errorMessage) {
	_dialogueLoaded = false;
	if (!_dialogue.load(decodedDialogScript, &_dialoguePresentation, errorMessage))
		return false;
	_dialogueLoaded = true;
	return true;
}

bool IntegratedGameplayHost::configureMotion(
		const ActorMotionClips &clips,
		Common::String &errorMessage) {
	return _motion.configure(clips, errorMessage);
}

void IntegratedGameplayHost::update(uint32 deltaMillis) {
	_motion.update(deltaMillis);
}

bool IntegratedGameplayHost::resolveEntityFloorPosition(
		const Common::String &entityName,
		NavVec2 &position) const {
	return _scene.resolveEntityFloorPosition(entityName, position);
}

void IntegratedGameplayHost::setActorFloorPosition(
		const Common::String &actorName,
		const NavVec2 &position) {
	_scene.setActorFloorPosition(actorName, position);
}

void IntegratedGameplayHost::setActorWalking(
		const Common::String &actorName,
		bool walking) {
	(void)actorName;

	// Motion mapping is deliberately optional until the retail SequenceTable
	// resolves semantic Walk/Standby states to concrete ANJ clips.
	if (_motion.configured() && !_motion.setWalking(walking)) {
		_lastError = Common::String::format(
			"Unable to enter actor motion state: %s", walking ? "walk" : "idle");
	}
}

bool IntegratedGameplayHost::startDialog(const Common::String &speaker,
                                         const Common::String &dialogName) {
	if (!_dialogueLoaded)
		return false;
	return _dialogue.startDialog(speaker, dialogName);
}

bool IntegratedGameplayHost::isDialogPlaying() const {
	return _dialogueLoaded && _dialogue.isDialogPlaying();
}

void IntegratedGameplayHost::showExamineText(const Common::String &text) {
	_scene.showExamineText(text);
}

} // End of namespace ZeroComico
