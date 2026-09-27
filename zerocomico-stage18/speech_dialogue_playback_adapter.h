/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE18_SPEECH_DIALOGUE_PLAYBACK_ADAPTER_H
#define ZEROCOMICO_STAGE18_SPEECH_DIALOGUE_PLAYBACK_ADAPTER_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage15/speech_aware_dialogue.h"
#include "zerocomico-stage18/speech_player.h"

namespace ZeroComico {

/**
 * Adapts the Stage 18 SpeechPlayer to the Stage 15 dialogue presentation.
 *
 * The path has already been resolved from an explicit retail index by
 * SpeechAwareDialoguePresentation. This adapter performs no enumeration.
 */
class SpeechPlayerDialoguePlaybackHost : public DialogueSpeechPlaybackHost {
public:
	explicit SpeechPlayerDialoguePlaybackHost(SpeechPlayer &player);

	bool playSpeech(const Common::Path &path) override;
	void stopSpeech() override;

	const Common::String &lastError() const { return _lastError; }

private:
	SpeechPlayer &_player;
	Common::String _lastError;
};

} // End of namespace ZeroComico

#endif
