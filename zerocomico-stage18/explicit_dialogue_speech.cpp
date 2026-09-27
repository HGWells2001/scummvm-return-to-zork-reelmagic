/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage18/explicit_dialogue_speech.h"

namespace ZeroComico {

ExplicitDialogueSpeech::ExplicitDialogueSpeech(
		const SpeechResourceCatalog &catalog,
		SpeechPlayer &player) :
	_catalog(catalog),
	_player(player) {
}

bool ExplicitDialogueSpeech::play(
		const Common::String &mainPlace,
		const Common::String &dialogueSpeaker,
		const Common::String &mainPlayerName,
		uint32 explicitRetailIndex,
		Common::String &errorMessage) {
	const DialogueSpeechResource resource =
		resolveDialogueSpeechResource(
			_catalog, mainPlace, dialogueSpeaker,
			mainPlayerName, explicitRetailIndex);

	if (!resource.available) {
		errorMessage = Common::String::format(
			"No retail speech sample for %s index %04u in %s",
			resource.effectiveSpeaker.c_str(),
			(uint)explicitRetailIndex,
			mainPlace.c_str());
		return false;
	}

	return _player.play(resource, errorMessage);
}

} // End of namespace ZeroComico
