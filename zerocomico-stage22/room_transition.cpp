/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage22/room_transition.h"

namespace ZeroComico {

void RoomTransitionGraph::clear() {
	_transitions.clear();
}

const ResolvedRoomTransition *RoomTransitionGraph::find(
		const Common::String &sourceRoom,
		const Common::String &portalShape) const {
	for (uint32 i = 0; i < _transitions.size(); ++i) {
		const ResolvedRoomTransition &transition = _transitions[i];
		if (transition.sourceRoom.equalsIgnoreCase(sourceRoom) &&
		    transition.portalShape.equalsIgnoreCase(portalShape))
			return &transition;
	}
	return nullptr;
}

void RoomTransitionGraphBuilder::build(
		const RoomTopologyDocument &rooms,
		const ShapeScriptDocument &shapes,
		const ShapeGeometryDocument *geometry,
		RoomTransitionGraph &out) const {
	out.clear();

	Common::Array<RoomPortalResolution> evidence;
	RoomPortalTopologyResolver resolver;
	resolver.resolve(rooms, shapes, evidence);

	for (uint32 i = 0; i < evidence.size(); ++i) {
		const RoomPortalResolution &resolved = evidence[i];
		if (!resolved.portal ||
		    !resolved.hasUniqueRoom() ||
		    !resolved.hasUniquePortalShape())
			continue;

		const ExtendedRoomRecord *target =
			rooms.room(resolved.matchingRooms[0]);
		if (!target)
			continue;

		ResolvedRoomTransition transition;
		transition.sourceRoom = resolved.ownerRoom;
		transition.targetRoom = target->name;
		transition.portalShape = resolved.matchingPortalShapes[0];

		transition.targetPrefix = target->prefix;
		transition.targetBackground = target->background;
		transition.targetObjects = target->objects;
		transition.targetMap = target->map;
		transition.targetCameraMap = target->cameraMap;
		transition.targetCamera = target->camera;
		transition.targetCameraSpot = target->cameraSpot;
		transition.rawPortalFields = resolved.portal->fields;

		if (geometry) {
			const ShapeGeometryRecord *portal =
				geometry->shape(transition.portalShape);
			if (portal && portal->kind == kShapeDefinitionPortal)
				transition.portalGeometry = portal;
		}

		out._transitions.push_back(transition);
	}
}

RoomTransitionRuntime::RoomTransitionRuntime() :
	_graph(nullptr),
	_host(nullptr) {
}

void RoomTransitionRuntime::bind(const RoomTransitionGraph *graph,
                                 RoomTransitionRuntimeHost *host) {
	_graph = graph;
	_host = host;
	_lastError.clear();
}

void RoomTransitionRuntime::clear() {
	_graph = nullptr;
	_host = nullptr;
	_currentRoom.clear();
	_lastError.clear();
}

RoomTransitionResult RoomTransitionRuntime::activatePortal(
		const Common::String &portalShape) {
	_lastError.clear();

	if (!_graph || !_host || _currentRoom.empty()) {
		_lastError = "Room transition runtime is not fully bound";
		return kRoomTransitionNotFound;
	}

	const ResolvedRoomTransition *transition =
		_graph->find(_currentRoom, portalShape);
	if (!transition) {
		_lastError = Common::String::format(
			"No unambiguous transition from Room '%s' through Portal '%s'",
			_currentRoom.c_str(), portalShape.c_str());
		return kRoomTransitionNotFound;
	}

	Common::String hostError;
	if (!_host->activateResolvedRoom(*transition, hostError)) {
		_lastError = hostError.empty()
			? Common::String::format(
				"Host rejected transition from '%s' to '%s'",
				transition->sourceRoom.c_str(),
				transition->targetRoom.c_str())
			: hostError;
		return kRoomTransitionHostRejected;
	}

	_currentRoom = transition->targetRoom;
	return kRoomTransitionDone;
}

} // End of namespace ZeroComico
