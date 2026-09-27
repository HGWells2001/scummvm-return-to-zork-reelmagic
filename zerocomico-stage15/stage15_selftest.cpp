/* Zero Comico Stage 15 speech catalog self-test. */

#include <cassert>

#include "zerocomico-stage15/dialogue_speech_resolver.h"
#include "zerocomico-stage15/speech_catalog.h"
#include "zerocomico-stage15/speech_aware_dialogue.h"

using namespace ZeroComico;

static void addMp1RetailSpeech(SpeechResourceCatalog &catalog) {
	assert(catalog.add(Common::Path("Speech/MP1/Aldo0000.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0000.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0001.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0002.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0003.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0004.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0005.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/Operaio0000.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/Operaio0001.mp3")));
	assert(catalog.add(Common::Path("Speech/MP1/Operaio0002.mp3")));
}

static void testFilenameParsing() {
	SpeechResourceId id;
	assert(parseSpeechResourcePath(
		Common::Path("program files/Medusa Games/Zero Comico/Speech/MP1/giovanni0005.mp3"),
		id));
	assert(id.mainPlace == "Mp1");
	assert(id.speakerStem == "giovanni");
	assert(id.index == 5);

	assert(parseSpeechResourcePath(
		Common::Path("Speech/Mp4/Giacomo0001.mp3"), id));
	assert(id.mainPlace == "Mp4");
	assert(id.speakerStem == "Giacomo");
	assert(id.index == 1);

	assert(!parseSpeechResourcePath(
		Common::Path("Speech/MP1/giovanni0000.mp3.sfk"), id));
	assert(!parseSpeechResourcePath(
		Common::Path("Speech/MP1/giovanni12.mp3"), id));
	assert(!parseSpeechResourcePath(
		Common::Path("Music/MP1/giovanni0000.mp3"), id));
}

static void testPathBuilder() {
	assert(buildSpeechResourcePath("mp1", "giovanni", 5).toString() ==
	       "Speech/Mp1/giovanni0005.mp3");
	assert(buildSpeechResourcePath("MP4", "Giacomo", 17).toString() ==
	       "Speech/Mp4/Giacomo0017.mp3");
	assert(buildSpeechResourcePath("Mp1", "x", 10000).empty());
}

static void testRetailMp1Catalog() {
	SpeechResourceCatalog catalog;
	addMp1RetailSpeech(catalog);

	assert(catalog.entries().size() == 10);
	assert(catalog.count("mp1", "GIOVANNI") == 6);
	assert(catalog.highestIndex("MP1", "Giovanni") == 5);
	assert(catalog.count("Mp1", "Operaio") == 3);
	assert(catalog.highestIndex("Mp1", "aldo") == 0);

	const SpeechResourceEntry *gio5 =
		catalog.find("Mp1", "Giovanni", 5);
	assert(gio5);
	assert(gio5->path.toString() ==
	       "Speech/MP1/giovanni0005.mp3");

	const SpeechResourceEntry *worker2 =
		catalog.find("mp1", "operaio", 2);
	assert(worker2);
	assert(worker2->path.toString() ==
	       "Speech/MP1/Operaio0002.mp3");

	assert(!catalog.find("Mp1", "Pacman", 0));
	assert(!catalog.find("Mp1", "Giovanni", 6));

	// Same logical resource with different casing must not duplicate.
	assert(catalog.add(Common::Path("Speech/Mp1/GIOVANNI0005.mp3")));
	assert(catalog.entries().size() == 10);
}

class FakeTextPresentation : public DialoguePresentationHost {
public:
	FakeTextPresentation() : speechCount(0), choiceCount(0) {}

	void showSpeech(const DialogueSpeaker &speaker,
	                const Common::String &text) override {
		lastSpeaker = speaker.name;
		lastText = text;
		++speechCount;
	}
	void hideSpeech() override {}
	void showChoices(const Common::Array<Common::String> &) override {
		++choiceCount;
	}
	void hideChoices() override {}

	int speechCount;
	int choiceCount;
	Common::String lastSpeaker;
	Common::String lastText;
};

class ExplicitTestIndexProvider : public DialogueSpeechIndexProvider {
public:
	bool speechIndex(const Common::String &mainPlace,
	                 const Common::String &dialogName,
	                 uint32 nodeIndex,
	                 const Common::String &speakerName,
	                 const Common::String &text,
	                 uint32 &index) const override {
		if (mainPlace.equalsIgnoreCase("Mp1") &&
		    dialogName.equalsIgnoreCase("Pacman_conpoz") &&
		    nodeIndex == 0 &&
		    speakerName.equalsIgnoreCase("Giovanni") &&
		    text == "Prima battuta") {
			index = 2;
			return true;
		}
		return false;
	}
};

class FakeSpeechAudio : public DialogueSpeechPlaybackHost {
public:
	FakeSpeechAudio() : playCount(0), stopCount(0) {}

	bool playSpeech(const Common::Path &path) override {
		lastPath = path;
		++playCount;
		return true;
	}
	void stopSpeech() override {
		++stopCount;
	}

	int playCount;
	int stopCount;
	Common::Path lastPath;
};

static void testSpeechAwareDialogue() {
	SpeechResourceCatalog catalog;
	assert(catalog.add(Common::Path("Speech/MP1/giovanni0002.mp3")));

	ExplicitTestIndexProvider indices;
	FakeSpeechAudio audio;
	FakeTextPresentation text;
	GameplayDialogueService service;

	SpeechAwareDialoguePresentation presentation(
		"Mp1", "Giovanni", catalog, indices, audio, text);
	presentation.bindService(&service);

	const char *script =
		"speaker Giovanni G 255 255 0 0.07\n"
		"speaker Pacman P 95 250 210 0.07\n"
		"Dialog Pacman_conpoz\n"
		"{\n"
		"G \"Prima battuta\"\n"
		"P \"Seconda battuta senza voce\"\n"
		"}\n";

	Common::String error;
	assert(service.load(script, &presentation, error));
	assert(service.startDialog("giovanni", "Pacman_conpoz"));

	assert(text.speechCount == 1);
	assert(text.lastSpeaker == "Giovanni");
	assert(audio.playCount == 1);
	assert(presentation.speechAudioPlaying());
	assert(presentation.lastSpeechPath().toString() ==
	       "Speech/MP1/giovanni0002.mp3");

	assert(service.advanceSpeech());
	assert(text.speechCount == 2);
	assert(text.lastSpeaker == "Pacman");
	assert(audio.playCount == 1);
	assert(!presentation.speechAudioPlaying());
	assert(presentation.lastSpeechPath().empty());

	assert(service.advanceSpeech());
	assert(!service.isDialogPlaying());
}

static void testExplicitDialogueResolution() {
	SpeechResourceCatalog catalog;
	addMp1RetailSpeech(catalog);

	DialogueSpeechResource direct = resolveDialogueSpeechResource(
		catalog, "Mp1", "Giovanni", "", 2);
	assert(direct.available);
	assert(direct.effectiveSpeaker == "Giovanni");
	assert(direct.explicitIndex == 2);
	assert(direct.path.toString() ==
	       "Speech/MP1/giovanni0002.mp3");

	DialogueSpeechResource alias = resolveDialogueSpeechResource(
		catalog, "Mp1", "MainPlayer", "Giovanni", 4);
	assert(alias.available);
	assert(alias.effectiveSpeaker == "Giovanni");
	assert(alias.path.toString() ==
	       "Speech/MP1/giovanni0004.mp3");

	DialogueSpeechResource unresolved = resolveDialogueSpeechResource(
		catalog, "Mp1", "MainPlayer", "", 0);
	assert(!unresolved.available);
}

int main() {
	testFilenameParsing();
	testPathBuilder();
	testRetailMp1Catalog();
	testExplicitDialogueResolution();
	testSpeechAwareDialogue();
	return 0;
}
