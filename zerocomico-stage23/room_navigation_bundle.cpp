/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage23/room_navigation_bundle.h"

#include "zerocomico-stage6/mainplace.h"

namespace ZeroComico {

namespace {

static Common::String ensureBspExtension(Common::String resource) {
	Common::String lower = resource;
	lower.toLowercase();
	if (!lower.hasSuffix(".bsp"))
		resource += ".bsp";
	return resource;
}

} // namespace

Common::Path roomBspResourcePath(const Common::String &mainPlaceName,
                                 const Common::String &resourceName) {
	Common::String resource = resourceName;
	resource.trim();
	if (resource.empty())
		return Common::Path();

	resource = ensureBspExtension(resource);

	if (resource.contains('/'))
		return Common::Path(resource);

	return Common::Path(Common::String::format(
		"%s/gameplay/%s",
		normalizeMainPlaceName(mainPlaceName).c_str(),
		resource.c_str()));
}

bool RoomNavigationBundleLoader::load(
		const Common::String &mainPlaceName,
		const ResolvedRoomTransition &transition,
		GameplayResourceHost &resources,
		PreparedRoomNavigation &out,
		Common::String &errorMessage) const {
	errorMessage.clear();
	PreparedRoomNavigation prepared;
	prepared.roomName = transition.targetRoom;

	if (transition.targetMap.empty()) {
		errorMessage = Common::String::format(
			"Target Room '%s' does not declare map:",
			transition.targetRoom.c_str());
		return false;
	}

	prepared.mapPath =
		roomBspResourcePath(mainPlaceName, transition.targetMap);
	if (prepared.mapPath.empty()) {
		errorMessage = "Unable to build target Map path";
		return false;
	}

	Common::String mapText;
	if (!resources.readPlainText(prepared.mapPath, mapText)) {
		errorMessage = Common::String::format(
			"Unable to read target actor Map %s",
			prepared.mapPath.toString().c_str());
		return false;
	}
	if (mapText.empty()) {
		errorMessage = Common::String::format(
			"Target actor Map %s is empty",
			prepared.mapPath.toString().c_str());
		return false;
	}
	if (!prepared.actorNavigation.parse(mapText, errorMessage)) {
		if (errorMessage.empty()) {
			errorMessage = Common::String::format(
				"Unable to parse target actor Map %s",
				prepared.mapPath.toString().c_str());
		}
		return false;
	}

	if (!transition.targetCameraMap.empty()) {
		prepared.cameraMapDeclared = true;
		prepared.cameraMapPath =
			roomBspResourcePath(mainPlaceName, transition.targetCameraMap);

		Common::String cameraText;
		if (!resources.readPlainText(prepared.cameraMapPath, cameraText)) {
			errorMessage = Common::String::format(
				"Unable to read target camera MapCam %s",
				prepared.cameraMapPath.toString().c_str());
			return false;
		}

		if (cameraText.empty()) {
			// Two retail MapCam files are genuinely zero bytes.
			prepared.cameraMapEmpty = true;
			prepared.cameraNavigationAvailable = false;
		} else {
			if (!prepared.cameraNavigation.parse(cameraText, errorMessage)) {
				if (errorMessage.empty()) {
					errorMessage = Common::String::format(
						"Unable to parse target camera MapCam %s",
						prepared.cameraMapPath.toString().c_str());
				}
				return false;
			}
			prepared.cameraNavigationAvailable = true;
		}
	}

	out = prepared;
	return true;
}

RoomNavigationTransitionHost::RoomNavigationTransitionHost() :
	_resources(nullptr),
	_activation(nullptr) {
}

void RoomNavigationTransitionHost::bind(
		const Common::String &mainPlaceName,
		GameplayResourceHost *resources,
		RoomNavigationActivationHost *activation) {
	_mainPlaceName = normalizeMainPlaceName(mainPlaceName);
	_resources = resources;
	_activation = activation;
}

void RoomNavigationTransitionHost::clear() {
	_mainPlaceName.clear();
	_resources = nullptr;
	_activation = nullptr;
	_current.clear();
}

bool RoomNavigationTransitionHost::activateResolvedRoom(
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) {
	errorMessage.clear();
	if (_mainPlaceName.empty() || !_resources || !_activation) {
		errorMessage = "RoomNavigationTransitionHost is not fully bound";
		return false;
	}

	PreparedRoomNavigation prepared;
	RoomNavigationBundleLoader loader;
	if (!loader.load(_mainPlaceName, transition, *_resources,
	                 prepared, errorMessage))
		return false;

	if (!_activation->activatePreparedRoom(
			transition, prepared, errorMessage))
		return false;

	_current = prepared;
	return true;
}

} // End of namespace ZeroComico
