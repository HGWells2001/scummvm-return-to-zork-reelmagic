/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE18_EXPLICIT_DIALOGUE_SPEECH_H
#define ZEROCOMICO_STAGE18_EXPLICIT_DIALOGUE_SPEECH_H

#include "common/str.h"

#include "zerocomico-stage15/dialogue_speech_resolver.h"
#include "zerocomico-stage18/speech_player.h"

namespace ZeroComico {

/**
 * Bridges a proven explicit speech index to actual playback.
 *
 * It deliberately has no API that accepts only a DialogueSpeech line:
 * callers must supply the retail index.
 */
class ExplicitDialogueSpeech {
public:
	ExplicitDialogueSpeech(const SpeechResourceCatalog &catalog,
	                       SpeechPlayer &player);

	bool play(const Common::String &mainPlace,
	          const Common::String &dialogueSpeaker,
	          const Common::String &mainPlayerName,
	          uint32 explicitRetailIndex,
	          Common::String &errorMessage);

	void stop() { _player.stop(); }
	bool isPlaying() const { return _player.isPlaying(); }

private:
	const SpeechResourceCatalog &_catalog;
	SpeechPlayer &_player;
};

} // End of namespace ZeroComico

#endif
