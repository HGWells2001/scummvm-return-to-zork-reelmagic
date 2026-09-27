/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage14/motion_runtime_evidence.h"

namespace ZeroComico {

namespace {

static bool containsClip(const Common::Array<AnimationClip> &clips,
                         const Common::String &name) {
	if (name.empty())
		return false;

	for (uint32 i = 0; i < clips.size(); ++i) {
		if (clips[i].name.equalsIgnoreCase(name))
			return true;
	}
	return false;
}

} // namespace

bool resolveAnimSetRuntimeEvidence(
		const AnimSetMotionProfile &profile,
		const SequenceTableDocument &sequenceTable,
		const Common::Array<AnimationClip> &animationClips,
		AnimSetRuntimeEvidence &out,
		Common::String &errorMessage) {
	out = AnimSetRuntimeEvidence();
	errorMessage.clear();

	out.standbyClip = profile.standbyAnimation;
	out.standbyAfterRunClip = profile.standbyAfterRunAnimation;
	out.walkSequence = profile.walkSequence;
	out.runSequence = profile.runSequence;

	out.standbyClipPresent =
		containsClip(animationClips, out.standbyClip);
	out.standbyAfterRunClipPresent =
		containsClip(animationClips, out.standbyAfterRunClip);

	if (!out.walkSequence.empty()) {
		out.walkSequenceMentioned =
			sequenceTable.containsIdentifier(out.walkSequence);
	}
	if (!out.runSequence.empty()) {
		out.runSequenceMentioned =
			sequenceTable.containsIdentifier(out.runSequence);
	}

	if (!out.standbyClip.empty() && !out.standbyClipPresent) {
		errorMessage = Common::String::format(
			"AnimSet standby animation '%s' is not present in decoded ANJ clips",
			out.standbyClip.c_str());
		return false;
	}

	if (!out.standbyAfterRunClip.empty() &&
	    !out.standbyAfterRunClipPresent) {
		errorMessage = Common::String::format(
			"AnimSet standby-after-run animation '%s' is not present in decoded ANJ clips",
			out.standbyAfterRunClip.c_str());
		return false;
	}

	if (!out.walkSequence.empty() && !out.walkSequenceMentioned) {
		errorMessage = Common::String::format(
			"AnimSet walk sequence '%s' is not referenced by the SequenceTable",
			out.walkSequence.c_str());
		return false;
	}

	if (!out.runSequence.empty() && !out.runSequenceMentioned) {
		errorMessage = Common::String::format(
			"AnimSet run sequence '%s' is not referenced by the SequenceTable",
			out.runSequence.c_str());
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
