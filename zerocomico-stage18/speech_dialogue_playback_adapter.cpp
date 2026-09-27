/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage18/speech_dialogue_playback_adapter.h"

namespace ZeroComico {

SpeechPlayerDialoguePlaybackHost::SpeechPlayerDialoguePlaybackHost(
		SpeechPlayer &player) :
	_player(player) {
}

bool SpeechPlayerDialoguePlaybackHost::playSpeech(
		const Common::Path &path) {
	_lastError.clear();

	DialogueSpeechResource resource;
	resource.available = !path.empty();
	resource.path = path;

	return _player.play(resource, _lastError);
}

void SpeechPlayerDialoguePlaybackHost::stopSpeech() {
	_player.stop();
}

} // End of namespace ZeroComico
