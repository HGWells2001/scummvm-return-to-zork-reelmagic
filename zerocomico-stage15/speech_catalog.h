/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE15_SPEECH_CATALOG_H
#define ZEROCOMICO_STAGE15_SPEECH_CATALOG_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

namespace ZeroComico {

struct SpeechResourceId {
	Common::String mainPlace;
	Common::String speakerStem;
	uint32 index;

	SpeechResourceId() : index(0) {}
};

struct SpeechResourceEntry {
	SpeechResourceId id;
	Common::Path path;
};

Common::String normalizeSpeechSpeakerKey(const Common::String &speaker);
Common::Path buildSpeechResourcePath(const Common::String &mainPlace,
                                     const Common::String &speakerStem,
                                     uint32 index);

bool parseSpeechResourcePath(const Common::Path &path,
                             SpeechResourceId &out);

/**
 * Catalog of retail speech files.
 *
 * Speaker and MainPlace lookup is case-insensitive because the shipped disc
 * mixes names such as MP1/Mp2 and Giovanni/giovanni/Giacomo/giacomo.
 */
class SpeechResourceCatalog {
public:
	void clear();

	bool add(const Common::Path &path);
	const SpeechResourceEntry *find(const Common::String &mainPlace,
	                                const Common::String &speaker,
	                                uint32 index) const;

	uint32 count(const Common::String &mainPlace,
	             const Common::String &speaker) const;
	int32 highestIndex(const Common::String &mainPlace,
	                   const Common::String &speaker) const;

	const Common::Array<SpeechResourceEntry> &entries() const {
		return _entries;
	}

private:
	Common::Array<SpeechResourceEntry> _entries;
};

/**
 * Resolves the logical speaker used by dialog.isc into the retail filename
 * stem. MainPlayer remains an engine-side alias because its concrete actor
 * depends on the active character.
 */
Common::String resolveSpeechSpeakerStem(const Common::String &dialogueSpeaker,
                                        const Common::String &mainPlayerName);

} // End of namespace ZeroComico

#endif
