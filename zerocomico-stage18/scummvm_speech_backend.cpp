/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "common/stream.h"

#include "zerocomico-stage18/scummvm_speech_backend.h"

namespace ZeroComico {

ScummVMSpeechBackend::ScummVMSpeechBackend(Audio::Mixer *mixer) :
	_mixer(mixer) {
}

ScummVMSpeechBackend::~ScummVMSpeechBackend() {
	stop();
}

bool ScummVMSpeechBackend::play(Common::SeekableReadStream *stream,
                                Common::String &errorMessage) {
	errorMessage.clear();
	if (!_mixer) {
		delete stream;
		errorMessage = "ScummVM speech backend has no mixer";
		return false;
	}
	if (!stream) {
		errorMessage = "ScummVM speech backend received a null stream";
		return false;
	}

#ifdef USE_MAD
	Audio::SeekableAudioStream *audio =
		Audio::makeMP3Stream(stream, DisposeAfterUse::YES);
	if (!audio) {
		errorMessage = "Unable to create MP3 decoder stream";
		return false;
	}

	stop();
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_handle, audio);
	return true;
#else
	delete stream;
	errorMessage = "ScummVM was built without MP3/MAD support";
	return false;
#endif
}

void ScummVMSpeechBackend::stop() {
	if (_mixer && _mixer->isSoundHandleActive(_handle))
		_mixer->stopHandle(_handle);
}

bool ScummVMSpeechBackend::isPlaying() const {
	return _mixer && _mixer->isSoundHandleActive(_handle);
}

} // End of namespace ZeroComico
