/* Zero Comico Stage 10 dialogue self-test. */

#include <cassert>

#include "zerocomico-stage10/dialogue_document.h"
#include "zerocomico-stage10/dialogue_mainplace_loader.h"
#include "zerocomico-stage10/dialogue_runtime.h"
#include "zerocomico-stage10/gameplay_dialogue_service.h"

using namespace ZeroComico;

static const char *kDialogScript =
	"speaker Giovanni G 255 255 0 0.07\n"
	"speaker Pacman P 95 250 210 0.07\n"
	"Dialog Pacman_conpoz\n"
	"{\n"
	"G \"Ehi, tutto bene?\"\n"
	"P \"No, per niente.\"\n"
	"}\n"
	"Dialog Scelta\n"
	"{\n"
	"G \"Che faccio?\"\n"
	"BEGIN\n"
	"\"Prima opzione\"\n"
	"\"Seconda opzione\"\n"
	"END\n"
	"}\n";

class FakePresentation : public DialoguePresentationHost {
public:
	FakePresentation() :
		speechVisible(false),
		choicesVisible(false),
		speechCount(0),
		choiceCount(0) {
	}

	void showSpeech(const DialogueSpeaker &speaker,
	                const Common::String &text) override {
		speechVisible = true;
		choicesVisible = false;
		lastSpeaker = speaker.name;
		lastText = text;
		++speechCount;
	}

	void hideSpeech() override {
		speechVisible = false;
	}

	void showChoices(const Common::Array<Common::String> &choices) override {
		choicesVisible = true;
		speechVisible = false;
		lastChoices = choices;
		++choiceCount;
	}

	void hideChoices() override {
		choicesVisible = false;
	}

	bool speechVisible;
	bool choicesVisible;
	int speechCount;
	int choiceCount;
	Common::String lastSpeaker;
	Common::String lastText;
	Common::Array<Common::String> lastChoices;
};

class FakeResources : public GameplayResourceHost {
public:
	bool readDecodedText(const Common::Path &path, Common::String &text) override {
		if (path.toString() != "Mp1/gameplay/dialog.isc")
			return false;
		text = kDialogScript;
		return true;
	}

	bool readPlainText(const Common::Path &, Common::String &) override {
		return false;
	}
};

static void testDocument() {
	DialogueDocument doc;
	DialogueDocumentParser parser;
	Common::String error;
	assert(parser.parse(kDialogScript, doc, error));
	assert(doc.speakers().size() == 2);
	assert(doc.dialogs().size() == 2);

	const DialogueSpeaker *gio = doc.speakerByName("Giovanni");
	assert(gio);
	assert(gio->key == "G");
	assert(gio->red == 255 && gio->green == 255 && gio->blue == 0);

	const DialogueBlock *pac = doc.dialog("Pacman_conpoz");
	assert(pac);
	assert(pac->nodes.size() == 2);
	assert(pac->nodes[0].type == kDialogueSpeechNode);
	assert(pac->nodes[0].speech.speakerName == "Giovanni");
	assert(pac->nodes[1].speech.speakerName == "Pacman");

	const DialogueBlock *choice = doc.dialog("Scelta");
	assert(choice);
	assert(choice->nodes.size() == 2);
	assert(choice->nodes[1].type == kDialogueChoiceNode);
	assert(choice->nodes[1].choice.choices.size() == 2);
}

static void testRuntime() {
	DialogueDocument doc;
	DialogueDocumentParser parser;
	Common::String error;
	assert(parser.parse(kDialogScript, doc, error));

	FakePresentation presentation;
	DialogueRuntime runtime;
	runtime.bind(&doc, &presentation);

	assert(runtime.start("giovanni", "Pacman_conpoz"));
	assert(runtime.isPlaying());
	assert(runtime.state() == kDialogueShowingSpeech);
	assert(presentation.lastSpeaker == "Giovanni");
	assert(presentation.lastText == "Ehi, tutto bene?");

	assert(runtime.advanceSpeech());
	assert(presentation.lastSpeaker == "Pacman");
	assert(runtime.advanceSpeech());
	assert(!runtime.isPlaying());
	assert(runtime.state() == kDialogueFinished);

	assert(runtime.start("giovanni", "Scelta"));
	assert(runtime.advanceSpeech());
	assert(runtime.state() == kDialogueWaitingChoice);
	assert(presentation.lastChoices.size() == 2);
	assert(runtime.selectChoice(1));
	assert(runtime.selectedChoice() == 1);
	assert(runtime.state() == kDialogueChoiceBranchUnresolved);
	assert(runtime.isPlaying());
}

static void testServiceAndLoader() {
	FakeResources resources;
	FakePresentation presentation;
	GameplayDialogueService service;
	DialogueMainPlaceLoader loader;
	Common::String error;

	assert(loader.load("Mp1", resources, &presentation, service, error));
	assert(gameplayDialogueScriptPath("mp1").toString() ==
	       "Mp1/gameplay/dialog.isc");
	assert(service.document().speakers().size() == 2);

	assert(service.startDialog("giovanni", "Pacman_conpoz"));
	assert(service.isDialogPlaying());
	assert(service.advanceSpeech());
	assert(service.advanceSpeech());
	assert(!service.isDialogPlaying());
}

int main() {
	testDocument();
	testRuntime();
	testServiceAndLoader();
	return 0;
}
