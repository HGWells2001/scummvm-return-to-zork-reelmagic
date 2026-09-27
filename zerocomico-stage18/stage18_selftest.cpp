/* Zero Comico Stage 18 speech playback self-test. */

#include <cassert>

#include "common/memstream.h"

#include "zerocomico-stage15/speech_catalog.h"
#include "zerocomico-stage18/explicit_dialogue_speech.h"
#include "zerocomico-stage18/speech_player.h"

using namespace ZeroComico;

class FakeStreamHost : public SpeechStreamHost {
public:
	FakeStreamHost() : opens(0), failOpen(false) {}

	Common::SeekableReadStream *openSpeechStream(
			const Common::Path &path) override {
		++opens;
		lastPath = path;
		if (failOpen)
			return nullptr;

		static const byte kData[] = {0x49, 0x44, 0x33, 0x00};
		return new Common::MemoryReadStream(
			kData, sizeof(kData), DisposeAfterUse::NO);
	}

	int opens;
	bool failOpen;
	Common::Path lastPath;
};

class FakePlaybackBackend : public SpeechPlaybackBackend {
public:
	FakePlaybackBackend() :
		playCalls(0),
		stopCalls(0),
		active(false),
		failPlay(false) {
	}

	bool play(Common::SeekableReadStream *stream,
	          Common::String &errorMessage) override {
		++playCalls;
		delete stream;

		if (failPlay) {
			active = false;
			errorMessage = "synthetic backend failure";
			return false;
		}

		active = true;
		errorMessage.clear();
		return true;
	}

	void stop() override {
		++stopCalls;
		active = false;
	}

	bool isPlaying() const override {
		return active;
	}

	int playCalls;
	int stopCalls;
	bool active;
	bool failPlay;
};

static SpeechResourceCatalog makeCatalog() {
	SpeechResourceCatalog catalog;
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0000.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0005.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/Operaio0002.mp3")));
	return catalog;
}

static void testResolvedPlayback() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	FakePlaybackBackend backend;
	SpeechPlayer player(streams, backend);
	ExplicitDialogueSpeech speech(catalog, player);

	Common::String error;
	assert(speech.play("Mp1", "MainPlayer", "Giovanni", 5, error));
	assert(error.empty());
	assert(streams.opens == 1);
	assert(streams.lastPath.toString() ==
	       "Speech/MP1/giovanni0005.mp3");
	assert(backend.playCalls == 1);
	assert(speech.isPlaying());
	assert(player.currentPath().toString() ==
	       "Speech/MP1/giovanni0005.mp3");

	speech.stop();
	assert(!speech.isPlaying());
	assert(backend.stopCalls == 1);
	assert(!player.hasCurrentResource());
}

static void testMissingIndexNeverOpensStream() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	FakePlaybackBackend backend;
	SpeechPlayer player(streams, backend);
	ExplicitDialogueSpeech speech(catalog, player);

	Common::String error;
	assert(!speech.play("Mp1", "Giovanni", "", 4, error));
	assert(!error.empty());
	assert(streams.opens == 0);
	assert(backend.playCalls == 0);
	assert(!speech.isPlaying());
}

static void testOpenFailure() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	streams.failOpen = true;
	FakePlaybackBackend backend;
	SpeechPlayer player(streams, backend);
	ExplicitDialogueSpeech speech(catalog, player);

	Common::String error;
	assert(!speech.play("Mp1", "Operaio", "", 2, error));
	assert(!error.empty());
	assert(streams.opens == 1);
	assert(backend.playCalls == 0);
	assert(!player.hasCurrentResource());
}

static void testBackendFailureOwnsStream() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	FakePlaybackBackend backend;
	backend.failPlay = true;
	SpeechPlayer player(streams, backend);
	ExplicitDialogueSpeech speech(catalog, player);

	Common::String error;
	assert(!speech.play("Mp1", "Giovanni", "", 0, error));
	assert(error == "synthetic backend failure");
	assert(streams.opens == 1);
	assert(backend.playCalls == 1);
	assert(!player.hasCurrentResource());
	assert(!speech.isPlaying());
}

static void testReplacingSpeechStopsPreviousHandle() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	FakePlaybackBackend backend;
	SpeechPlayer player(streams, backend);
	ExplicitDialogueSpeech speech(catalog, player);

	Common::String error;
	assert(speech.play("Mp1", "Giovanni", "", 0, error));
	assert(backend.stopCalls == 0);

	assert(speech.play("Mp1", "Giovanni", "", 5, error));
	assert(backend.stopCalls == 1);
	assert(backend.playCalls == 2);
	assert(player.currentPath().toString() ==
	       "Speech/MP1/giovanni0005.mp3");
}

int main() {
	testResolvedPlayback();
	testMissingIndexNeverOpensStream();
	testOpenFailure();
	testBackendFailureOwnsStream();
	testReplacingSpeechStopsPreviousHandle();
	return 0;
}
