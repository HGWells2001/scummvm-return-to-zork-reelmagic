/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE18_SPEECH_PLAYER_H
#define ZEROCOMICO_STAGE18_SPEECH_PLAYER_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage15/dialogue_speech_resolver.h"

namespace Common {
class SeekableReadStream;
}

namespace ZeroComico {

class SpeechStreamHost {
public:
	virtual ~SpeechStreamHost() {}

	/**
	 * Return a newly allocated stream. Ownership is transferred to the
	 * playback backend when play() succeeds; otherwise the caller disposes it.
	 */
	virtual Common::SeekableReadStream *openSpeechStream(
		const Common::Path &path) = 0;
};

class SpeechPlaybackBackend {
public:
	virtual ~SpeechPlaybackBackend() {}

	virtual bool play(Common::SeekableReadStream *stream,
	                  Common::String &errorMessage) = 0;
	virtual void stop() = 0;
	virtual bool isPlaying() const = 0;
};

/**
 * High-level speech player.
 *
 * It accepts only a DialogueSpeechResource that Stage 15 already resolved
 * from an explicit retail speech index. It never enumerates dialogue lines.
 */
class SpeechPlayer {
public:
	SpeechPlayer(SpeechStreamHost &streams,
	             SpeechPlaybackBackend &backend);

	bool play(const DialogueSpeechResource &resource,
	          Common::String &errorMessage);
	void stop();

	bool isPlaying() const;
	const Common::Path &currentPath() const { return _currentPath; }
	bool hasCurrentResource() const { return !_currentPath.empty(); }

private:
	SpeechStreamHost &_streams;
	SpeechPlaybackBackend &_backend;
	Common::Path _currentPath;
};

} // End of namespace ZeroComico

#endif
