/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage15/speech_aware_dialogue.h"

namespace ZeroComico {

SpeechAwareDialoguePresentation::SpeechAwareDialoguePresentation(
		const Common::String &mainPlace,
		const Common::String &mainPlayerName,
		const SpeechResourceCatalog &catalog,
		const DialogueSpeechIndexProvider &indices,
		DialogueSpeechPlaybackHost &audio,
		DialoguePresentationHost &textPresentation) :
	_mainPlace(mainPlace),
	_mainPlayerName(mainPlayerName),
	_catalog(catalog),
	_indices(indices),
	_audio(audio),
	_textPresentation(textPresentation),
	_service(nullptr),
	_speechAudioPlaying(false) {
}

void SpeechAwareDialoguePresentation::bindService(
		const GameplayDialogueService *service) {
	_service = service;
}

void SpeechAwareDialoguePresentation::showSpeech(
		const DialogueSpeaker &speaker,
		const Common::String &text) {
	_textPresentation.showSpeech(speaker, text);

	if (_speechAudioPlaying) {
		_audio.stopSpeech();
		_speechAudioPlaying = false;
	}
	_lastSpeechPath = Common::Path();

	if (!_service)
		return;

	const DialogueRuntime &runtime = _service->runtime();
	uint32 explicitIndex = 0;
	if (!_indices.speechIndex(_mainPlace,
	                          runtime.dialogName(),
	                          runtime.nodeIndex(),
	                          speaker.name,
	                          text,
	                          explicitIndex)) {
		return;
	}

	const DialogueSpeechResource resource =
		resolveDialogueSpeechResource(_catalog,
		                              _mainPlace,
		                              speaker.name,
		                              _mainPlayerName,
		                              explicitIndex);
	if (!resource.available)
		return;

	_lastSpeechPath = resource.path;
	_speechAudioPlaying = _audio.playSpeech(resource.path);
}

void SpeechAwareDialoguePresentation::hideSpeech() {
	if (_speechAudioPlaying)
		_audio.stopSpeech();
	_speechAudioPlaying = false;
	_lastSpeechPath = Common::Path();
	_textPresentation.hideSpeech();
}

void SpeechAwareDialoguePresentation::showChoices(
		const Common::Array<Common::String> &choices) {
	if (_speechAudioPlaying)
		_audio.stopSpeech();
	_speechAudioPlaying = false;
	_lastSpeechPath = Common::Path();
	_textPresentation.showChoices(choices);
}

void SpeechAwareDialoguePresentation::hideChoices() {
	_textPresentation.hideChoices();
}

} // End of namespace ZeroComico
