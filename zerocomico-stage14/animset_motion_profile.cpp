/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage14/animset_motion_profile.h"

namespace ZeroComico {

namespace {

static void copyPair(const CharacterAnimSetField *field,
                     Common::String &first,
                     Common::String &second) {
	if (!field)
		return;
	if (!field->values.empty())
		first = field->values[0];
	if (field->values.size() > 1)
		second = field->values[1];
}

static bool rejectExcess(const CharacterAnimSet &animSet,
                         const CharacterAnimSetField *field,
                         uint32 maximum,
                         Common::String &errorMessage) {
	if (!field || field->values.size() <= maximum)
		return false;

	errorMessage = Common::String::format(
		"AnimSet '%s' field '%s' has %u values; Stage 14 has evidence for at most %u",
		animSet.name.c_str(), field->key.c_str(),
		(uint)field->values.size(), (uint)maximum);
	return true;
}

} // namespace

bool buildAnimSetMotionProfile(const CharacterAnimSet &animSet,
                               AnimSetMotionProfile &out,
                               Common::String &errorMessage) {
	out = AnimSetMotionProfile();
	errorMessage.clear();

	const CharacterAnimSetField *standby = animSet.field("standby");
	const CharacterAnimSetField *blend = animSet.field("Blend");
	const CharacterAnimSetField *turn = animSet.field("turn");
	const CharacterAnimSetField *walk = animSet.field("walk");

	if (rejectExcess(animSet, standby, 2, errorMessage) ||
	    rejectExcess(animSet, blend, 2, errorMessage) ||
	    rejectExcess(animSet, turn, 2, errorMessage) ||
	    rejectExcess(animSet, walk, 2, errorMessage))
		return false;

	copyPair(standby, out.standbyAnimation, out.standbyAfterRunAnimation);
	copyPair(blend, out.blendStartAnimation, out.blendEndAnimation);
	copyPair(turn, out.turnLeftAnimation, out.turnRightAnimation);
	copyPair(walk, out.walkSequence, out.runSequence);

	if (out.empty()) {
		errorMessage = Common::String::format(
			"AnimSet '%s' has no proven locomotion fields",
			animSet.name.c_str());
		return false;
	}

	return true;
}

AnimSetSequenceEvidence correlateMotionSequences(
		const AnimSetMotionProfile &profile,
		const SequenceTableDocument &sequenceTable) {
	AnimSetSequenceEvidence evidence;
	if (!profile.walkSequence.empty())
		evidence.walkSequenceMentioned =
			sequenceTable.containsIdentifier(profile.walkSequence);
	if (!profile.runSequence.empty())
		evidence.runSequenceMentioned =
			sequenceTable.containsIdentifier(profile.runSequence);
	return evidence;
}

} // End of namespace ZeroComico
