/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE18_SCUMMVM_SPEECH_BACKEND_H
#define ZEROCOMICO_STAGE18_SCUMMVM_SPEECH_BACKEND_H

#include "audio/mixer.h"

#include "zerocomico-stage18/speech_player.h"

namespace ZeroComico {

/**
 * Native ScummVM MP3 speech backend.
 *
 * MP3 decoding is available when ScummVM was built with USE_MAD. If not,
 * play() fails explicitly and ownership of the input stream stays with the
 * caller.
 */
class ScummVMSpeechBackend : public SpeechPlaybackBackend {
public:
	explicit ScummVMSpeechBackend(Audio::Mixer *mixer);
	~ScummVMSpeechBackend() override;

	bool play(Common::SeekableReadStream *stream,
	          Common::String &errorMessage) override;
	void stop() override;
	bool isPlaying() const override;

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
};

} // End of namespace ZeroComico

#endif
