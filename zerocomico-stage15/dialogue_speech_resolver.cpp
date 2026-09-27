/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage15/dialogue_speech_resolver.h"

namespace ZeroComico {

DialogueSpeechResource resolveDialogueSpeechResource(
		const SpeechResourceCatalog &catalog,
		const Common::String &mainPlace,
		const Common::String &dialogueSpeaker,
		const Common::String &mainPlayerName,
		uint32 explicitIndex) {
	DialogueSpeechResource result;
	result.effectiveSpeaker =
		resolveSpeechSpeakerStem(dialogueSpeaker, mainPlayerName);
	result.explicitIndex = explicitIndex;

	const SpeechResourceEntry *entry =
		catalog.find(mainPlace, result.effectiveSpeaker, explicitIndex);
	if (!entry)
		return result;

	result.path = entry->path;
	result.available = true;
	return result;
}

} // End of namespace ZeroComico
