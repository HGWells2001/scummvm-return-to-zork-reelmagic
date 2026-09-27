/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE23_GAMEPLAY_ROOM_TRANSITION_HOST_H
#define ZEROCOMICO_STAGE23_GAMEPLAY_ROOM_TRANSITION_HOST_H

#include "common/str.h"

#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage22/room_transition.h"

namespace ZeroComico {

/**
 * Applies a resolved Stage 22 room transition to the live gameplay state.
 *
 * Only room-local state changes:
 *   - active Room descriptor;
 *   - BSP/navigation graph.
 *
 * MainPlace-wide puzzle Objects and variables remain untouched.
 *
 * The target BSP is parsed into a temporary object first. The live state is
 * committed only after all validation succeeds.
 */
class GameplayRoomTransitionHost : public RoomTransitionRuntimeHost {
public:
	GameplayRoomTransitionHost();

	void bind(GameplayMainPlaceState *state,
	          GameplayResourceHost *resources);
	void clear();

	bool activateResolvedRoom(
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) override;

private:
	bool validateTransitionDescriptor(
		const GameplayRoomDescriptor &room,
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) const;

	GameplayMainPlaceState *_state;
	GameplayResourceHost *_resources;
};

} // End of namespace ZeroComico

#endif
