/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage21/room_topology.h"

namespace ZeroComico {

namespace {

static Common::String cleanToken(Common::String token) {
	token.trim();
	while (!token.empty() &&
	       (token.lastChar() == ':' || token.lastChar() == ';' ||
	        token.lastChar() == '{' || token.lastChar() == '}')) {
		token.deleteLastChar();
	}
	return token;
}

static Common::String stripComment(const Common::String &line) {
	bool inQuote = false;
	for (uint32 i = 0; i + 1 < line.size(); ++i) {
		if (line[i] == '"')
			inQuote = !inQuote;
		if (!inQuote && line[i] == '/' && line[i + 1] == '/') {
			Common::String result = line.substr(0, i);
			result.trim();
			return result;
		}
	}
	Common::String result = line;
	result.trim();
	return result;
}

static int braceDelta(const Common::String &line) {
	bool inQuote = false;
	int delta = 0;
	for (uint32 i = 0; i < line.size(); ++i) {
		const char c = line[i];
		if (c == '"') {
			inQuote = !inQuote;
			continue;
		}
		if (inQuote)
			continue;
		if (c == '{')
			++delta;
		else if (c == '}')
			--delta;
	}
	return delta;
}

static void tokenizeQuoted(const Common::String &line,
                           Common::Array<Common::String> &tokens) {
	tokens.clear();

	Common::String current;
	bool inQuote = false;

	for (uint32 i = 0; i < line.size(); ++i) {
		const char c = line[i];

		if (c == '"') {
			inQuote = !inQuote;
			continue;
		}

		if (!inQuote && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
			if (!current.empty()) {
				tokens.push_back(cleanToken(current));
				current.clear();
			}
			continue;
		}

		if (!inQuote && (c == '{' || c == '}')) {
			if (!current.empty()) {
				tokens.push_back(cleanToken(current));
				current.clear();
			}
			continue;
		}

		current += c;
	}

	if (!current.empty())
		tokens.push_back(cleanToken(current));

	for (int32 i = (int32)tokens.size() - 1; i >= 0; --i) {
		if (tokens[i].empty())
			tokens.remove_at(i);
	}
}

static Common::String valueAfterKey(const Common::Array<Common::String> &tokens) {
	if (tokens.size() < 2)
		return Common::String();
	return tokens[1];
}

static bool containsIgnoreCase(const Common::Array<Common::String> &values,
                               const Common::String &value) {
	for (uint32 i = 0; i < values.size(); ++i) {
		if (values[i].equalsIgnoreCase(value))
			return true;
	}
	return false;
}

} // namespace

void RoomTopologyDocument::clear() {
	_rooms.clear();
}

const ExtendedRoomRecord *RoomTopologyDocument::room(
		const Common::String &name) const {
	for (uint32 i = 0; i < _rooms.size(); ++i) {
		if (_rooms[i].name.equalsIgnoreCase(name))
			return &_rooms[i];
	}
	return nullptr;
}

bool RoomTopologyParser::parse(const Common::String &decodedRoomScript,
                               RoomTopologyDocument &out,
                               Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decodedRoomScript.empty()) {
		errorMessage = "Room script is empty";
		return false;
	}

	uint32 physicalLine = 1;
	uint32 lineStart = 0;
	int depth = 0;
	int roomBaseDepth = -1;
	int32 currentRoom = -1;

	while (lineStart <= decodedRoomScript.size()) {
		uint32 lineEnd = lineStart;
		while (lineEnd < decodedRoomScript.size() &&
		       decodedRoomScript[lineEnd] != '\r' &&
		       decodedRoomScript[lineEnd] != '\n') {
			++lineEnd;
		}

		const Common::String raw =
			decodedRoomScript.substr(lineStart, lineEnd - lineStart);
		const Common::String code = stripComment(raw);

		if (!code.empty()) {
			Common::Array<Common::String> tokens;
			tokenizeQuoted(code, tokens);

			if (currentRoom < 0 &&
			    !tokens.empty() &&
			    tokens[0].equalsIgnoreCase("Room")) {
				if (tokens.size() < 2) {
					errorMessage = Common::String::format(
						"Room on line %u has no name",
						(uint)physicalLine);
					return false;
				}

				ExtendedRoomRecord room;
				room.lineNumber = physicalLine;
				room.name = tokens[1];
				out._rooms.push_back(room);
				currentRoom = (int32)out._rooms.size() - 1;
				roomBaseDepth = depth;
			} else if (currentRoom >= 0 && !tokens.empty()) {
				ExtendedRoomRecord &room = out._rooms[currentRoom];
				Common::String key = tokens[0];
				key.toLowercase();

				if (key == "prefix") {
					room.prefix = valueAfterKey(tokens);
				} else if (key == "backgrd") {
					room.background = valueAfterKey(tokens);
				} else if (key == "objects") {
					room.objects = valueAfterKey(tokens);
				} else if (key == "map") {
					room.map = valueAfterKey(tokens);
				} else if (key == "cameramap") {
					room.cameraMap = valueAfterKey(tokens);
				} else if (key == "camera") {
					room.camera = valueAfterKey(tokens);
				} else if (key == "cameraspot") {
					room.cameraSpot = valueAfterKey(tokens);
				} else if (key == "portal") {
					if (tokens.size() != 6) {
						errorMessage = Common::String::format(
							"Room '%s' portal on line %u has %u fields; retail serializer proves 5",
							room.name.c_str(), (uint)physicalLine,
							(uint)(tokens.size() > 0 ? tokens.size() - 1 : 0));
						return false;
					}

					RoomPortalRecord portal;
					portal.lineNumber = physicalLine;
					portal.rawLine = raw;
					for (uint32 i = 1; i < tokens.size(); ++i)
						portal.fields.push_back(tokens[i]);
					room.portals.push_back(portal);
				}
			}

			depth += braceDelta(code);
			if (depth < 0) {
				errorMessage = Common::String::format(
					"Room script closes too many braces on line %u",
					(uint)physicalLine);
				return false;
			}

			if (currentRoom >= 0 &&
			    roomBaseDepth >= 0 &&
			    depth <= roomBaseDepth &&
			    code.contains("}")) {
				currentRoom = -1;
				roomBaseDepth = -1;
			}
		}

		if (lineEnd >= decodedRoomScript.size())
			break;

		if (decodedRoomScript[lineEnd] == '\r' &&
		    lineEnd + 1 < decodedRoomScript.size() &&
		    decodedRoomScript[lineEnd + 1] == '\n') {
			lineStart = lineEnd + 2;
		} else {
			lineStart = lineEnd + 1;
		}
		++physicalLine;
	}

	if (depth != 0) {
		errorMessage = "Room script has unbalanced braces";
		return false;
	}
	if (out._rooms.empty()) {
		errorMessage = "No Room declarations found";
		return false;
	}

	return true;
}

void RoomPortalTopologyResolver::resolve(
		const RoomTopologyDocument &rooms,
		const ShapeScriptDocument &shapes,
		Common::Array<RoomPortalResolution> &out) const {
	out.clear();

	for (uint32 r = 0; r < rooms.rooms().size(); ++r) {
		const ExtendedRoomRecord &owner = rooms.rooms()[r];

		for (uint32 p = 0; p < owner.portals.size(); ++p) {
			const RoomPortalRecord &portal = owner.portals[p];

			RoomPortalResolution resolution;
			resolution.ownerRoom = owner.name;
			resolution.portalIndex = p;
			resolution.portal = &portal;

			for (uint32 f = 0; f < portal.fields.size(); ++f) {
				const Common::String &field = portal.fields[f];

				for (uint32 rr = 0; rr < rooms.rooms().size(); ++rr) {
					const Common::String &candidate = rooms.rooms()[rr].name;
					if (candidate.equalsIgnoreCase(owner.name))
						continue;
					if (field.equalsIgnoreCase(candidate) &&
					    !containsIgnoreCase(resolution.matchingRooms, candidate)) {
						resolution.matchingRooms.push_back(candidate);
					}
				}

				const ShapeDefinition *shape = shapes.shape(field);
				if (shape && shape->kind == kShapeDefinitionPortal &&
				    !containsIgnoreCase(resolution.matchingPortalShapes, shape->name)) {
					resolution.matchingPortalShapes.push_back(shape->name);
				}
			}

			out.push_back(resolution);
		}
	}
}

} // End of namespace ZeroComico
