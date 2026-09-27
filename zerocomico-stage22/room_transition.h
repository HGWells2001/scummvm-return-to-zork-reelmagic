/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE22_ROOM_TRANSITION_H
#define ZEROCOMICO_STAGE22_ROOM_TRANSITION_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage16/shape_document.h"
#include "zerocomico-stage19/shape_geometry.h"
#include "zerocomico-stage21/room_topology.h"

namespace ZeroComico {

struct ResolvedRoomTransition {
	Common::String sourceRoom;
	Common::String targetRoom;
	Common::String portalShape;

	Common::String targetPrefix;
	Common::String targetBackground;
	Common::String targetObjects;
	Common::String targetMap;
	Common::String targetCameraMap;
	Common::String targetCamera;
	Common::String targetCameraSpot;

	Common::Array<Common::String> rawPortalFields;
	const ShapeGeometryRecord *portalGeometry;

	ResolvedRoomTransition() :
		portalGeometry(nullptr) {}
};

class RoomTransitionGraph {
public:
	void clear();

	const ResolvedRoomTransition *find(const Common::String &sourceRoom,
	                                   const Common::String &portalShape) const;

	const Common::Array<ResolvedRoomTransition> &transitions() const {
		return _transitions;
	}

private:
	friend class RoomTransitionGraphBuilder;
	Common::Array<ResolvedRoomTransition> _transitions;
};

/**
 * Builds only transitions whose Stage 21 evidence is unambiguous:
 * exactly one other Room name and exactly one Portal Shape occur among the
 * five retail portal fields.
 *
 * Numeric Portal A/B geometry is attached when Stage 19 has it, but is not
 * required and no A/B semantics are assigned.
 */
class RoomTransitionGraphBuilder {
public:
	void build(const RoomTopologyDocument &rooms,
	           const ShapeScriptDocument &shapes,
	           const ShapeGeometryDocument *geometry,
	           RoomTransitionGraph &out) const;
};

class RoomTransitionRuntimeHost {
public:
	virtual ~RoomTransitionRuntimeHost() {}

	/**
	 * Must transactionally validate/load all target-room resources and only
	 * return true when the transition can safely become current.
	 */
	virtual bool activateResolvedRoom(
		const ResolvedRoomTransition &transition,
		Common::String &errorMessage) = 0;
};

enum RoomTransitionResult {
	kRoomTransitionDone,
	kRoomTransitionNotFound,
	kRoomTransitionHostRejected
};

class RoomTransitionRuntime {
public:
	RoomTransitionRuntime();

	void bind(const RoomTransitionGraph *graph,
	          RoomTransitionRuntimeHost *host);

	void clear();

	void setCurrentRoom(const Common::String &roomName) {
		_currentRoom = roomName;
	}

	const Common::String &currentRoom() const { return _currentRoom; }
	const Common::String &lastError() const { return _lastError; }

	RoomTransitionResult activatePortal(const Common::String &portalShape);

private:
	const RoomTransitionGraph *_graph;
	RoomTransitionRuntimeHost *_host;
	Common::String _currentRoom;
	Common::String _lastError;
};

} // End of namespace ZeroComico

#endif
