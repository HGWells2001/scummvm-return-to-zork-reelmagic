/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE14_ANIMSET_MOTION_PROFILE_H
#define ZEROCOMICO_STAGE14_ANIMSET_MOTION_PROFILE_H

#include "common/str.h"

#include "zerocomico-stage13/sequence_table.h"
#include "zerocomico-stage14/character_animset.h"

namespace ZeroComico {

struct AnimSetMotionProfile {
	Common::String standbyAnimation;
	Common::String standbyAfterRunAnimation;
	Common::String blendStartAnimation;
	Common::String blendEndAnimation;
	Common::String turnLeftAnimation;
	Common::String turnRightAnimation;
	Common::String walkSequence;
	Common::String runSequence;

	bool empty() const {
		return standbyAnimation.empty() &&
		       standbyAfterRunAnimation.empty() &&
		       walkSequence.empty() &&
		       runSequence.empty();
	}
};

struct AnimSetSequenceEvidence {
	bool walkSequenceMentioned;
	bool runSequenceMentioned;

	AnimSetSequenceEvidence() :
		walkSequenceMentioned(false),
		runSequenceMentioned(false) {}
};

/**
 * The field arities below are grounded by adjacent retail executable
 * diagnostics:
 *
 * standby: -> standby animation, standby-after-run animation
 * Blend:   -> blend-start animation, blend-end animation
 * turn:    -> turn-left animation, turn-right animation
 * walk:    -> walk animseq, run animseq
 */
bool buildAnimSetMotionProfile(const CharacterAnimSet &animSet,
                               AnimSetMotionProfile &out,
                               Common::String &errorMessage);

AnimSetSequenceEvidence correlateMotionSequences(
	const AnimSetMotionProfile &profile,
	const SequenceTableDocument &sequenceTable);

} // End of namespace ZeroComico

#endif
