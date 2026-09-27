/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE15_DIALOGUE_SPEECH_RESOLVER_H
#define ZEROCOMICO_STAGE15_DIALOGUE_SPEECH_RESOLVER_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage15/speech_catalog.h"

namespace ZeroComico {

struct DialogueSpeechResource {
	Common::String effectiveSpeaker;
	uint32 explicitIndex;
	Common::Path path;
	bool available;

	DialogueSpeechResource() :
		explicitIndex(0), available(false) {}
};

/**
 * Resolves a speech path only when the caller already has an explicit retail
 * speech index. Stage 15 intentionally does not infer that index from dialog
 * line order.
 */
DialogueSpeechResource resolveDialogueSpeechResource(
	const SpeechResourceCatalog &catalog,
	const Common::String &mainPlace,
	const Common::String &dialogueSpeaker,
	const Common::String &mainPlayerName,
	uint32 explicitIndex);

} // End of namespace ZeroComico

#endif
