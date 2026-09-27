/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE14_MOTION_RUNTIME_EVIDENCE_H
#define ZEROCOMICO_STAGE14_MOTION_RUNTIME_EVIDENCE_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage13/sequence_table.h"
#include "zerocomico-stage14/animset_motion_profile.h"

namespace ZeroComico {

/**
 * Evidence that can be proven today from an AnimSet + ANJ + SequenceTable.
 *
 * Direct standby fields name animations; walk/run fields name JACS sequences.
 * The latter must not be fed directly to ActorMotionController until the
 * SequenceTable transition graph resolves them to ANJ clips.
 */
struct AnimSetRuntimeEvidence {
	Common::String standbyClip;
	Common::String standbyAfterRunClip;
	Common::String walkSequence;
	Common::String runSequence;

	bool standbyClipPresent;
	bool standbyAfterRunClipPresent;
	bool walkSequenceMentioned;
	bool runSequenceMentioned;

	AnimSetRuntimeEvidence() :
		standbyClipPresent(false),
		standbyAfterRunClipPresent(false),
		walkSequenceMentioned(false),
		runSequenceMentioned(false) {}

	bool hasDirectIdle() const {
		return standbyClipPresent && !standbyClip.empty();
	}

	bool hasResolvedWalkClip() const {
		// Intentionally false until JACS transition semantics are decoded.
		return false;
	}
};

bool resolveAnimSetRuntimeEvidence(
	const AnimSetMotionProfile &profile,
	const SequenceTableDocument &sequenceTable,
	const Common::Array<AnimationClip> &animationClips,
	AnimSetRuntimeEvidence &out,
	Common::String &errorMessage);

} // End of namespace ZeroComico

#endif
