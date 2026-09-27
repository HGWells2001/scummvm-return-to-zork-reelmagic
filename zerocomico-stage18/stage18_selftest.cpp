/* Zero Comico Stage 18 speech playback self-test. */

#include <cassert>

#include "common/memstream.h"

#include "zerocomico-stage15/speech_catalog.h"
#include "zerocomico-stage18/explicit_dialogue_speech.h"
#include "zerocomico-stage18/speech_player.h"
#include "zerocomico-stage18/speech_dialogue_playback_adapter.h"
#include "zerocomico-stage15/speech_aware_dialogue.h"
#include "zerocomico-stage10/gameplay_dialogue_service.h"

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

class FakeTextPresentation : public DialoguePresentationHost {
public:
	FakeTextPresentation() : speechCount(0) {}

	void showSpeech(const DialogueSpeaker &speaker,
	                const Common::String &text) override {
		lastSpeaker = speaker.name;
		lastText = text;
		++speechCount;
	}
	void hideSpeech() override {}
	void showChoices(const Common::Array<Common::String> &) override {}
	void hideChoices() override {}

	int speechCount;
	Common::String lastSpeaker;
	Common::String lastText;
};

class TestSpeechIndexProvider : public DialogueSpeechIndexProvider {
public:
	bool speechIndex(const Common::String &mainPlace,
	                 const Common::String &dialogName,
	                 uint32 nodeIndex,
	                 const Common::String &speakerName,
	                 const Common::String &text,
	                 uint32 &index) const override {
		if (mainPlace.equalsIgnoreCase("Mp1") &&
		    dialogName.equalsIgnoreCase("TestDialog") &&
		    nodeIndex == 0 &&
		    speakerName.equalsIgnoreCase("Giovanni") &&
		    text == "Voce reale") {
			index = 5;
			return true;
		}
		return false;
	}
};

static SpeechResourceCatalog makeCatalog() {
	SpeechResourceCatalog catalog;
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0000.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0005.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/Operaio0002.mp3")));
	return catalog;
}

static void testEndToEndDialoguePlayback() {
	SpeechResourceCatalog catalog = makeCatalog();
	FakeStreamHost streams;
	FakePlaybackBackend backend;
	SpeechPlayer player(streams, backend);
	SpeechPlayerDialoguePlaybackHost audio(player);

	TestSpeechIndexProvider indices;
	FakeTextPresentation text;
	GameplayDialogueService service;
	SpeechAwareDialoguePresentation presentation(
		"Mp1", "Giovanni", catalog, indices, audio, text);
	presentation.bindService(&service);

	const char *dialogScript =
		"speaker Giovanni G 255 255 0 0.07\n"
		"Dialog TestDialog\n"
		"{\n"
		"G \"Voce reale\"\n"
		"}\n";

	Common::String error;
	assert(service.load(dialogScript, &presentation, error));
	assert(service.startDialog("Giovanni", "TestDialog"));
	assert(text.speechCount == 1);
	assert(text.lastSpeaker == "Giovanni");
	assert(text.lastText == "Voce reale");
	assert(streams.opens == 1);
	assert(streams.lastPath.toString() ==
	       "Speech/MP1/giovanni0005.mp3");
	assert(backend.playCalls == 1);
	assert(backend.active);
	assert(audio.lastError().empty());

	assert(service.advanceSpeech());
	assert(!service.isDialogPlaying());
	assert(!backend.active);
	assert(backend.stopCalls == 1);
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
	testEndToEndDialoguePlayback();
	testResolvedPlayback();
	testMissingIndexNeverOpensStream();
	testOpenFailure();
	testBackendFailureOwnsStream();
	testReplacingSpeechStopsPreviousHandle();
	return 0;
}
