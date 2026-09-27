/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE21_ROOM_TOPOLOGY_H
#define ZEROCOMICO_STAGE21_ROOM_TOPOLOGY_H

#include "common/array.h"
#include "common/str.h"

#include "zerocomico-stage16/shape_document.h"

namespace ZeroComico {

struct RoomPortalRecord {
	uint32 lineNumber;
	Common::Array<Common::String> fields;
	Common::String rawLine;

	RoomPortalRecord() : lineNumber(0) {}
};

struct ExtendedRoomRecord {
	uint32 lineNumber;
	Common::String name;
	Common::String prefix;
	Common::String background;
	Common::String objects;
	Common::String map;
	Common::String cameraMap;
	Common::String camera;
	Common::String cameraSpot;
	Common::Array<RoomPortalRecord> portals;

	ExtendedRoomRecord() : lineNumber(0) {}
};

class RoomTopologyDocument {
public:
	void clear();

	const ExtendedRoomRecord *room(const Common::String &name) const;
	const Common::Array<ExtendedRoomRecord> &rooms() const { return _rooms; }

private:
	friend class RoomTopologyParser;
	Common::Array<ExtendedRoomRecord> _rooms;
};

/**
 * Line-oriented parser for the room.isc subset whose serialization is proven
 * by strings in the retail executable.
 *
 * Proven fields:
 *   Prefix:
 *   backgrd:
 *   objects:
 *   portal: %s %s %s "%s" %s
 *   map:
 *   cameramap:
 *   camera:
 *   cameraspot:
 */
class RoomTopologyParser {
public:
	bool parse(const Common::String &decodedRoomScript,
	           RoomTopologyDocument &out,
	           Common::String &errorMessage) const;
};

struct RoomPortalResolution {
	Common::String ownerRoom;
	uint32 portalIndex;
	const RoomPortalRecord *portal;
	Common::Array<Common::String> matchingRooms;
	Common::Array<Common::String> matchingPortalShapes;

	RoomPortalResolution() :
		portalIndex(0),
		portal(nullptr) {}

	bool hasUniqueRoom() const { return matchingRooms.size() == 1; }
	bool hasUniquePortalShape() const { return matchingPortalShapes.size() == 1; }
};

class RoomPortalTopologyResolver {
public:
	void resolve(const RoomTopologyDocument &rooms,
	             const ShapeScriptDocument &shapes,
	             Common::Array<RoomPortalResolution> &out) const;
};

} // End of namespace ZeroComico

#endif
