/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE13_ACTOR_SEQUENCE_TABLE_H
#define ZEROCOMICO_STAGE13_ACTOR_SEQUENCE_TABLE_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage8/shared_actor.h"
#include "zerocomico-stage9/actor_model.h"
#include "zerocomico-stage13/sequence_table.h"

namespace ZeroComico {

struct SequenceClipReference {
	Common::String clipName;
	uint32 lineNumber;
	SequenceDirectiveKind directiveKind;
	SequenceTransitionClass explicitTransition;
	Common::String rawLine;

	SequenceClipReference() :
		lineNumber(0),
		directiveKind(kSequenceDirectiveUnknown),
		explicitTransition(kSequenceTransitionUnknown) {}
};

struct SequenceClipCorrelation {
	Common::Array<SequenceClipReference> references;
	Common::Array<Common::String> referencedClips;
	Common::Array<Common::String> unreferencedClips;

	void clear() {
		references.clear();
		referencedClips.clear();
		unreferencedClips.clear();
	}
};

/**
 * Loads the actor .seq from its proven JFX1/LZHUF text container.
 */
class ActorSequenceTableLoader {
public:
	bool loadPacked(const SharedActorAssets &assets,
	                ActorPackedResourceHost &host,
	                SequenceTableDocument &out,
	                Common::String &errorMessage) const;
};

/**
 * Correlates exact identifiers present in a SequenceTable with already
 * decoded ANJ clip names. This is evidence gathering, not semantic mapping.
 */
class SequenceClipCorrelator {
public:
	void correlate(const SequenceTableDocument &document,
	               const Common::Array<AnimationClip> &clips,
	               SequenceClipCorrelation &out) const;
};

} // End of namespace ZeroComico

#endif
