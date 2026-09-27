/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "common/stream.h"

#include "zerocomico-stage18/speech_player.h"

namespace ZeroComico {

SpeechPlayer::SpeechPlayer(SpeechStreamHost &streams,
                           SpeechPlaybackBackend &backend) :
	_streams(streams),
	_backend(backend) {
}

bool SpeechPlayer::play(const DialogueSpeechResource &resource,
                        Common::String &errorMessage) {
	errorMessage.clear();
	stop();

	if (!resource.available || resource.path.empty()) {
		errorMessage = "Speech resource is unresolved";
		return false;
	}

	Common::SeekableReadStream *stream =
		_streams.openSpeechStream(resource.path);
	if (!stream) {
		errorMessage = Common::String::format(
			"Unable to open speech resource %s",
			resource.path.toString().c_str());
		return false;
	}

	if (!_backend.play(stream, errorMessage)) {
		delete stream;
		return false;
	}

	_currentPath = resource.path;
	return true;
}

void SpeechPlayer::stop() {
	if (_backend.isPlaying())
		_backend.stop();
	_currentPath = Common::Path();
}

bool SpeechPlayer::isPlaying() const {
	return _backend.isPlaying();
}

} // End of namespace ZeroComico
