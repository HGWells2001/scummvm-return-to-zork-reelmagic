/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE9_ANJ_TRACKS_H
#define ZEROCOMICO_STAGE9_ANJ_TRACKS_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage6/animation_player.h"
#include "zerocomico-stage9/anj_document.h"
#include "zerocomico-stage9/p3d_model.h"

namespace ZeroComico {

enum ANJTargetKind {
	kANJTargetUnknown,
	kANJTargetTransform,
	kANJTargetCameraTarget,
	kANJTargetCamera,
	kANJTargetLight,
	kANJTargetSingleVec3
};

struct ANJDecodeStats {
	uint32 timelines;
	uint32 transformTargets;
	uint32 cameraTargets;
	uint32 cameras;
	uint32 lights;
	uint32 singleVec3Targets;
	uint32 unresolvedTargets;

	ANJDecodeStats();
};

class ANJTrackDecoder {
public:
	bool decode(const P3DModel &model,
	            const ANJDocument &document,
	            Common::Array<AnimationClip> &clips,
	            ANJDecodeStats &stats,
	            Common::String &errorMessage) const;
};

} // End of namespace ZeroComico

#endif
