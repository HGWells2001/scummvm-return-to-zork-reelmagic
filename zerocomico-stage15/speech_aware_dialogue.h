/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE15_SPEECH_AWARE_DIALOGUE_H
#define ZEROCOMICO_STAGE15_SPEECH_AWARE_DIALOGUE_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage10/dialogue_runtime.h"
#include "zerocomico-stage10/gameplay_dialogue_service.h"
#include "zerocomico-stage15/dialogue_speech_resolver.h"
#include "zerocomico-stage15/speech_catalog.h"

namespace ZeroComico {

/**
 * Supplies a retail speech index only when it is actually known.
 *
 * Implementations may come from a decoded SpeechTracer map, save-state
 * metadata, or another proven source. Returning false means "text only".
 */
class DialogueSpeechIndexProvider {
public:
	virtual ~DialogueSpeechIndexProvider() {}

	virtual bool speechIndex(const Common::String &mainPlace,
	                         const Common::String &dialogName,
	                         uint32 nodeIndex,
	                         const Common::String &speakerName,
	                         const Common::String &text,
	                         uint32 &index) const = 0;
};

class DialogueSpeechPlaybackHost {
public:
	virtual ~DialogueSpeechPlaybackHost() {}

	virtual bool playSpeech(const Common::Path &path) = 0;
	virtual void stopSpeech() = 0;
};

/**
 * Decorates the Stage 10 text presentation with optional retail speech.
 *
 * Subtitle/text presentation is always delegated. Audio is attempted only
 * when the index provider explicitly resolves the current dialogue node.
 */
class SpeechAwareDialoguePresentation : public DialoguePresentationHost {
public:
	SpeechAwareDialoguePresentation(
		const Common::String &mainPlace,
		const Common::String &mainPlayerName,
		const SpeechResourceCatalog &catalog,
		const DialogueSpeechIndexProvider &indices,
		DialogueSpeechPlaybackHost &audio,
		DialoguePresentationHost &textPresentation);

	void bindService(const GameplayDialogueService *service);

	void showSpeech(const DialogueSpeaker &speaker,
	                const Common::String &text) override;
	void hideSpeech() override;
	void showChoices(const Common::Array<Common::String> &choices) override;
	void hideChoices() override;

	bool speechAudioPlaying() const { return _speechAudioPlaying; }
	const Common::Path &lastSpeechPath() const { return _lastSpeechPath; }

private:
	Common::String _mainPlace;
	Common::String _mainPlayerName;
	const SpeechResourceCatalog &_catalog;
	const DialogueSpeechIndexProvider &_indices;
	DialogueSpeechPlaybackHost &_audio;
	DialoguePresentationHost &_textPresentation;
	const GameplayDialogueService *_service;
	bool _speechAudioPlaying;
	Common::Path _lastSpeechPath;
};

} // End of namespace ZeroComico

#endif
