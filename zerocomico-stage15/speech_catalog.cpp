/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage15/speech_catalog.h"

#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

namespace {

static bool endsWithIgnoreCase(const Common::String &value,
                               const char *suffix) {
	Common::String a = value;
	Common::String b(suffix);
	a.toLowercase();
	b.toLowercase();
	return a.hasSuffix(b);
}

static bool isFourDigits(const Common::String &value) {
	if (value.size() != 4)
		return false;
	for (uint32 i = 0; i < value.size(); ++i) {
		if (value[i] < '0' || value[i] > '9')
			return false;
	}
	return true;
}

static uint32 parseFourDigits(const Common::String &value) {
	uint32 result = 0;
	for (uint32 i = 0; i < value.size(); ++i)
		result = result * 10 + (uint32)(value[i] - '0');
	return result;
}

static Common::String normalizedMainPlace(const Common::String &value) {
	return normalizeMainPlaceName(value);
}

} // namespace

Common::String normalizeSpeechSpeakerKey(const Common::String &speaker) {
	Common::String result = speaker;
	result.trim();
	result.toLowercase();
	return result;
}

Common::Path buildSpeechResourcePath(const Common::String &mainPlace,
                                     const Common::String &speakerStem,
                                     uint32 index) {
	if (mainPlace.empty() || speakerStem.empty() || index > 9999)
		return Common::Path();

	const Common::String mp = normalizedMainPlace(mainPlace);
	return Common::Path(Common::String::format(
		"Speech/%s/%s%04u.mp3",
		mp.c_str(), speakerStem.c_str(), (uint)index));
}

bool parseSpeechResourcePath(const Common::Path &path,
                             SpeechResourceId &out) {
	out = SpeechResourceId();

	Common::String text = path.toString('/');
	if (!endsWithIgnoreCase(text, ".mp3"))
		return false;

	// We only need the final Speech/<MainPlace>/<filename> tail.
	Common::String normalized = text;
	for (uint32 i = 0; i < normalized.size(); ++i) {
		if (normalized[i] == '\\')
			normalized.setChar('/', i);
	}

	int32 lastSlash = -1;
	for (uint32 i = 0; i < normalized.size(); ++i) {
		if (normalized[i] == '/')
			lastSlash = (int32)i;
	}
	if (lastSlash < 0)
		return false;

	const Common::String filename = normalized.substr(lastSlash + 1);
	if (filename.size() < 8)
		return false;

	Common::String mpAndParent = normalized.substr(0, lastSlash);
	int32 mpSlash = -1;
	for (uint32 i = 0; i < mpAndParent.size(); ++i) {
		if (mpAndParent[i] == '/')
			mpSlash = (int32)i;
	}
	if (mpSlash < 0)
		return false;

	const Common::String mainPlace = mpAndParent.substr(mpSlash + 1);
	Common::String parent = mpAndParent.substr(0, mpSlash);

	int32 speechSlash = -1;
	for (uint32 i = 0; i < parent.size(); ++i) {
		if (parent[i] == '/')
			speechSlash = (int32)i;
	}
	const Common::String parentName =
		speechSlash >= 0 ? parent.substr(speechSlash + 1) : parent;
	if (!parentName.equalsIgnoreCase("Speech"))
		return false;

	const Common::String base = filename.substr(0, filename.size() - 4);
	if (base.size() < 5)
		return false;

	const Common::String digits = base.substr(base.size() - 4);
	if (!isFourDigits(digits))
		return false;

	const Common::String speaker = base.substr(0, base.size() - 4);
	if (speaker.empty())
		return false;

	out.mainPlace = normalizedMainPlace(mainPlace);
	out.speakerStem = speaker;
	out.index = parseFourDigits(digits);
	return true;
}

void SpeechResourceCatalog::clear() {
	_entries.clear();
}

bool SpeechResourceCatalog::add(const Common::Path &path) {
	SpeechResourceId id;
	if (!parseSpeechResourcePath(path, id))
		return false;

	// Avoid duplicate logical entries caused only by casing/path spelling.
	if (find(id.mainPlace, id.speakerStem, id.index))
		return true;

	SpeechResourceEntry entry;
	entry.id = id;
	entry.path = path;
	_entries.push_back(entry);
	return true;
}

const SpeechResourceEntry *SpeechResourceCatalog::find(
		const Common::String &mainPlace,
		const Common::String &speaker,
		uint32 index) const {
	const Common::String mp = normalizedMainPlace(mainPlace);
	const Common::String speakerKey = normalizeSpeechSpeakerKey(speaker);

	for (uint32 i = 0; i < _entries.size(); ++i) {
		const SpeechResourceEntry &entry = _entries[i];
		if (entry.id.index == index &&
		    entry.id.mainPlace.equalsIgnoreCase(mp) &&
		    normalizeSpeechSpeakerKey(entry.id.speakerStem) == speakerKey)
			return &entry;
	}
	return nullptr;
}

uint32 SpeechResourceCatalog::count(const Common::String &mainPlace,
                                    const Common::String &speaker) const {
	const Common::String mp = normalizedMainPlace(mainPlace);
	const Common::String speakerKey = normalizeSpeechSpeakerKey(speaker);
	uint32 result = 0;

	for (uint32 i = 0; i < _entries.size(); ++i) {
		if (_entries[i].id.mainPlace.equalsIgnoreCase(mp) &&
		    normalizeSpeechSpeakerKey(_entries[i].id.speakerStem) == speakerKey)
			++result;
	}
	return result;
}

int32 SpeechResourceCatalog::highestIndex(const Common::String &mainPlace,
                                          const Common::String &speaker) const {
	const Common::String mp = normalizedMainPlace(mainPlace);
	const Common::String speakerKey = normalizeSpeechSpeakerKey(speaker);
	int32 highest = -1;

	for (uint32 i = 0; i < _entries.size(); ++i) {
		const SpeechResourceEntry &entry = _entries[i];
		if (entry.id.mainPlace.equalsIgnoreCase(mp) &&
		    normalizeSpeechSpeakerKey(entry.id.speakerStem) == speakerKey &&
		    (int32)entry.id.index > highest) {
			highest = (int32)entry.id.index;
		}
	}
	return highest;
}

Common::String resolveSpeechSpeakerStem(const Common::String &dialogueSpeaker,
                                        const Common::String &mainPlayerName) {
	if (dialogueSpeaker.equalsIgnoreCase("MainPlayer") &&
	    !mainPlayerName.empty())
		return mainPlayerName;
	return dialogueSpeaker;
}

} // End of namespace ZeroComico
