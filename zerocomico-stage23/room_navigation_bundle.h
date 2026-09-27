/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE23_ROOM_NAVIGATION_BUNDLE_H
#define ZEROCOMICO_STAGE23_ROOM_NAVIGATION_BUNDLE_H

#include "common/path.h"
#include "common/str.h"

#include "zerocomico-stage7/bsp_navigation.h"
#include "zerocomico-stage7/gameplay_mainplace.h"
#include "zerocomico-stage22/room_transition.h"

namespace ZeroComico {

struct PreparedRoomNavigation {
	Common::String roomName;
	Common::Path mapPath;
	Common::Path cameraMapPath;

	BspNavigation actorNavigation;
	BspNavigation cameraNavigation;

	bool cameraMapDeclared;
	bool cameraMapEmpty;
	bool cameraNavigationAvailable;

	PreparedRoomNavigation() :
		cameraMapDeclared(false),
		cameraMapEmpty(false),
		cameraNavigationAvailable(false) {}

	void clear() {
		roomName.clear();
		mapPath = Common::Path();
		cameraMapPath = Common::Path();
		actorNavigation.clear();
		cameraNavigation.clear();
		cameraMapDeclared = false;
		cameraMapEmpty = false;
		cameraNavigationAvailable = false;
	}
};

Common::Path roomBspResourcePath(const Common::String &mainPlaceName,
                                 const Common::String &resourceName);

/**
 * Loads the walk Map and camera MapCam as two independent BSP plans.
 *
 * A declared zero-byte MapCam is valid retail data and is represented as
 * cameraMapEmpty=true, cameraNavigationAvailable=false.
 *
 * A missing/non-readable declared resource remains an error.
 */
class RoomNavigationBundleLoader {
public:
	bool load(const Common::String &mainPlaceName,
	          const ResolvedRoomTransition &transition,
	          GameplayResourceHost &resources,
	          PreparedRoomNavigation &out,
	          Common::String &errorMessage) const;
};

class RoomNavigationActivationHost {
public:
	virtual ~RoomNavigationActivationHost() {}

	/**
	 * The engine can prepare scene/camera resources here. Returning false
	 * aborts the room transition; PreparedRoomNavigation remains uncommitted.
	 */
	virtual bool activatePreparedRoom(
		const ResolvedRoomTransition &transition,
		const PreparedRoomNavigation &navigation,
		Common::String &errorMessage) = 0;
};

/**
 * Concrete Stage 22 host that prepares Map/MapCam transactionally before the
 * room transition is allowed to commit.
 */
class RoomNavigationTransitionHost : public RoomTransitionRuntimeHost {
public:
	RoomNavigationTransitionHost();

	void bind(const Common::String &mainPlaceName,
	          GameplayResourceHost *resources,
	          RoomNavigationActivationHost *activation);

	void clear();

	bool activateResolvedRoom(
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) override;

	const PreparedRoomNavigation &currentNavigation() const {
		return _current;
	}

private:
	Common::String _mainPlaceName;
	GameplayResourceHost *_resources;
	RoomNavigationActivationHost *_activation;
	PreparedRoomNavigation _current;
};

} // End of namespace ZeroComico

#endif
