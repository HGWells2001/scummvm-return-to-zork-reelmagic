/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cstdio>

#include "zerocomico-stage7/bsp_navigation.h"

#include "common/tokenizer.h"

#include <cfloat>
#include <cmath>
#include <cstdio>

namespace ZeroComico {

namespace {

class LineReader {
public:
	explicit LineReader(const Common::String &text) : _index(0) {
		Common::StringTokenizer tok(text, "\r\n");
		while (!tok.empty()) {
			Common::String line = tok.nextToken();
			line.trim();
			if (!line.empty())
				_lines.push_back(line);
		}
	}

	bool eos() const { return _index >= _lines.size(); }

	Common::String next() {
		if (eos())
			return Common::String();
		return _lines[_index++];
	}

	Common::String peek() const {
		if (eos())
			return Common::String();
		return _lines[_index];
	}

	bool expect(const char *value, Common::String &error) {
		const Common::String got = next();
		if (!got.equalsIgnoreCase(value)) {
			error = Common::String::format("Expected '%s', got '%s'", value, got.c_str());
			return false;
		}
		return true;
	}

private:
	Common::Array<Common::String> _lines;
	uint32 _index;
};

static bool parseInt(const Common::String &line, int32 &value) {
	int v = 0;
	if (sscanf(line.c_str(), "%d", &v) != 1)
		return false;
	value = v;
	return true;
}

static bool parseVec2(const Common::String &line, NavVec2 &value) {
	return sscanf(line.c_str(), "%f %f", &value.x, &value.y) == 2;
}

static bool parseFourInts(const Common::String &line, BspEdge &edge) {
	int a = 0, b = 0, c = 0, d = 0;
	if (sscanf(line.c_str(), "%d %d %d %d", &a, &b, &c, &d) != 4)
		return false;
	edge.point0 = a;
	edge.point1 = b;
	edge.frontCell = c;
	edge.backCell = d;
	return true;
}

static float distanceSquared(const NavVec2 &a, const NavVec2 &b) {
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	return dx * dx + dy * dy;
}

static bool parseTree(LineReader &reader, int32 &nodes, int32 &nulls,
                      Common::Array<int32> &leaves, Common::String &error) {
	if (reader.eos()) {
		error = "BSP tree ended unexpectedly";
		return false;
	}

	const Common::String line = reader.next();
	if (line == "-1") {
		++nulls;
		return true;
	}

	int edge = -1, leaf = -1, unused = 0;
	if (sscanf(line.c_str(), "%d %d %d", &edge, &leaf, &unused) != 3) {
		error = Common::String::format("Invalid BSP tree node '%s'", line.c_str());
		return false;
	}

	(void)edge;
	(void)unused;
	++nodes;
	if (leaf >= 0)
		leaves.push_back(leaf);

	return parseTree(reader, nodes, nulls, leaves, error) &&
	       parseTree(reader, nodes, nulls, leaves, error);
}

} // namespace

BspNavigation::BspNavigation() : _treeNodes(0), _treeNulls(0) {
}

void BspNavigation::clear() {
	_outline.clear();
	_holes.clear();
	_points.clear();
	_edges.clear();
	_cells.clear();
	_graph.clear();
	_treeLeaves.clear();
	_treeNodes = 0;
	_treeNulls = 0;
}

bool BspNavigation::parse(const Common::String &text, Common::String &errorMessage) {
	clear();
	errorMessage.clear();
	LineReader reader(text);

	if (!reader.expect("scene", errorMessage) ||
	    !reader.expect("room", errorMessage) ||
	    !reader.expect("poly", errorMessage))
		return false;

	int32 count = 0;
	if (!parseInt(reader.next(), count) || count < 0) {
		errorMessage = "Invalid room-outline vertex count";
		return false;
	}
	for (int32 i = 0; i < count; ++i) {
		NavVec2 p;
		if (!parseVec2(reader.next(), p)) {
			errorMessage = "Invalid room-outline vertex";
			return false;
		}
		_outline.push_back(p);
	}

	int32 holes = 0;
	if (!parseInt(reader.next(), holes) || holes < 0) {
		errorMessage = "Invalid room-hole count";
		return false;
	}
	for (int32 h = 0; h < holes; ++h) {
		if (!reader.expect("poly", errorMessage))
			return false;
		int32 vertices = 0;
		if (!parseInt(reader.next(), vertices) || vertices < 0) {
			errorMessage = "Invalid hole vertex count";
			return false;
		}
		Common::Array<NavVec2> polygon;
		for (int32 i = 0; i < vertices; ++i) {
			NavVec2 p;
			if (!parseVec2(reader.next(), p)) {
				errorMessage = "Invalid hole vertex";
				return false;
			}
			polygon.push_back(p);
		}
		_holes.push_back(polygon);
	}

	if (!reader.expect("scene_end", errorMessage) ||
	    !reader.expect("bsp", errorMessage) ||
	    !reader.expect("bsp_points", errorMessage))
		return false;

	int32 pointCount = 0;
	if (!parseInt(reader.next(), pointCount) || pointCount < 0) {
		errorMessage = "Invalid BSP point count";
		return false;
	}
	for (int32 i = 0; i < pointCount; ++i) {
		NavVec2 p;
		if (!parseVec2(reader.next(), p)) {
			errorMessage = "Invalid BSP point";
			return false;
		}
		_points.push_back(p);
	}

	if (!reader.expect("bsp_edges", errorMessage))
		return false;
	int32 edgeCount = 0;
	if (!parseInt(reader.next(), edgeCount) || edgeCount < 0) {
		errorMessage = "Invalid BSP edge count";
		return false;
	}
	for (int32 i = 0; i < edgeCount; ++i) {
		BspEdge edge;
		if (!parseFourInts(reader.next(), edge)) {
			errorMessage = "Invalid BSP edge";
			return false;
		}
		_edges.push_back(edge);
	}

	if (!reader.expect("bsp_polygons", errorMessage))
		return false;
	int32 cellCount = 0;
	if (!parseInt(reader.next(), cellCount) || cellCount < 0) {
		errorMessage = "Invalid BSP cell count";
		return false;
	}
	for (int32 i = 0; i < cellCount; ++i) {
		BspCell cell;
		int32 cellEdgeCount = 0;
		if (!parseInt(reader.next(), cell.index) ||
		    !parseInt(reader.next(), cellEdgeCount) || cellEdgeCount < 0) {
			errorMessage = "Invalid BSP cell";
			return false;
		}
		for (int32 e = 0; e < cellEdgeCount; ++e) {
			int32 edgeIndex = -1;
			if (!parseInt(reader.next(), edgeIndex)) {
				errorMessage = "Invalid BSP cell edge";
				return false;
			}
			cell.edgeIndices.push_back(edgeIndex);
		}
		_cells.push_back(cell);
	}

	if (!reader.expect("bsp_tree", errorMessage) ||
	    !parseTree(reader, _treeNodes, _treeNulls, _treeLeaves, errorMessage) ||
	    !reader.expect("bsp_end", errorMessage) ||
	    !reader.expect("pathfinding", errorMessage) ||
	    !reader.expect("graph", errorMessage))
		return false;

	int32 graphCount = 0;
	if (!parseInt(reader.next(), graphCount) || graphCount < 0) {
		errorMessage = "Invalid path graph node count";
		return false;
	}

	for (int32 i = 0; i < graphCount; ++i) {
		PathNode node;
		if (!parseVec2(reader.next(), node.position)) {
			errorMessage = "Invalid path graph position";
			return false;
		}

		Common::StringTokenizer arcTokens(reader.next());
		while (!arcTokens.empty()) {
			const Common::String nodeToken = arcTokens.nextToken();
			int32 target = -1;
			if (!parseInt(nodeToken, target)) {
				errorMessage = "Invalid path graph target";
				return false;
			}
			if (target == -1)
				break;
			if (arcTokens.empty()) {
				errorMessage = "Missing path graph weight";
				return false;
			}
			float weight = 0.0f;
			if (sscanf(arcTokens.nextToken().c_str(), "%f", &weight) != 1) {
				errorMessage = "Invalid path graph weight";
				return false;
			}
			node.arcs.push_back(PathArc(target, weight));
		}
		_graph.push_back(node);
	}

	if (!reader.expect("support", errorMessage))
		return false;
	int32 supportCount = 0;
	if (!parseInt(reader.next(), supportCount) || supportCount < 0) {
		errorMessage = "Invalid support-record count";
		return false;
	}
	for (int32 i = 0; i < supportCount; ++i) {
		int32 inside = 0;
		if (!parseInt(reader.next(), inside) || inside < 0) {
			errorMessage = "Invalid support inside-count";
			return false;
		}
		for (int32 j = 0; j < inside; ++j)
			reader.next();

		int32 weighted = 0;
		if (!parseInt(reader.next(), weighted) || weighted < 0) {
			errorMessage = "Invalid support weighted-count";
			return false;
		}
		for (int32 j = 0; j < weighted; ++j)
			reader.next();
	}

	if (!reader.expect("pathfinding_end", errorMessage))
		return false;

	return validate(errorMessage);
}

bool BspNavigation::validate(Common::String &errorMessage) const {
	if (_treeNulls != _treeNodes + 1) {
		errorMessage = "BSP preorder tree does not close";
		return false;
	}

	for (uint32 i = 0; i < _edges.size(); ++i) {
		if (_edges[i].point0 < 0 || _edges[i].point1 < 0 ||
		    (uint32)_edges[i].point0 >= _points.size() ||
		    (uint32)_edges[i].point1 >= _points.size()) {
			errorMessage = "BSP edge references an invalid point";
			return false;
		}
	}

	for (uint32 i = 0; i < _cells.size(); ++i) {
		for (uint32 e = 0; e < _cells[i].edgeIndices.size(); ++e) {
			const int32 edge = _cells[i].edgeIndices[e];
			if (edge < 0 || (uint32)edge >= _edges.size()) {
				errorMessage = "BSP cell references an invalid edge";
				return false;
			}
		}
	}

	Common::Array<bool> seenLeaves;
	seenLeaves.resize(_cells.size(), false);
	for (uint32 i = 0; i < _treeLeaves.size(); ++i) {
		const int32 leaf = _treeLeaves[i];
		if (leaf < 0 || (uint32)leaf >= _cells.size()) {
			errorMessage = "BSP tree references an invalid cell";
			return false;
		}
		if (seenLeaves[leaf]) {
			errorMessage = "BSP tree references a cell more than once";
			return false;
		}
		seenLeaves[leaf] = true;
	}

	for (uint32 i = 0; i < _graph.size(); ++i) {
		for (uint32 a = 0; a < _graph[i].arcs.size(); ++a) {
			const PathArc &arc = _graph[i].arcs[a];
			if (arc.node < 0 || (uint32)arc.node >= _graph.size() || arc.weight < 0.0f) {
				errorMessage = "Path graph contains an invalid arc";
				return false;
			}
		}
	}

	errorMessage.clear();
	return true;
}

int32 BspNavigation::nearestGraphNode(const NavVec2 &point) const {
	if (_graph.empty())
		return -1;

	int32 best = 0;
	float bestDistance = distanceSquared(point, _graph[0].position);
	for (uint32 i = 1; i < _graph.size(); ++i) {
		const float d = distanceSquared(point, _graph[i].position);
		if (d < bestDistance) {
			bestDistance = d;
			best = (int32)i;
		}
	}
	return best;
}

bool BspNavigation::findPath(const NavVec2 &start, const NavVec2 &goal,
                             Common::Array<NavVec2> &outPath) const {
	outPath.clear();
	const int32 source = nearestGraphNode(start);
	const int32 target = nearestGraphNode(goal);
	if (source < 0 || target < 0)
		return false;

	const uint32 n = _graph.size();
	Common::Array<float> distance;
	Common::Array<int32> previous;
	Common::Array<bool> visited;
	distance.resize(n, FLT_MAX);
	previous.resize(n, -1);
	visited.resize(n, false);
	distance[source] = 0.0f;

	for (uint32 iteration = 0; iteration < n; ++iteration) {
		int32 current = -1;
		float best = FLT_MAX;
		for (uint32 i = 0; i < n; ++i) {
			if (!visited[i] && distance[i] < best) {
				best = distance[i];
				current = (int32)i;
			}
		}
		if (current < 0)
			break;
		if (current == target)
			break;

		visited[current] = true;
		for (uint32 a = 0; a < _graph[current].arcs.size(); ++a) {
			const PathArc &arc = _graph[current].arcs[a];
			const float candidate = distance[current] + arc.weight;
			if (candidate < distance[arc.node]) {
				distance[arc.node] = candidate;
				previous[arc.node] = current;
			}
		}
	}

	if (source != target && previous[target] < 0)
		return false;

	Common::Array<int32> reverse;
	for (int32 node = target; node >= 0; node = previous[node]) {
		reverse.push_back(node);
		if (node == source)
			break;
	}
	if (reverse.empty() || reverse.back() != source)
		return false;

	outPath.push_back(start);
	for (int32 i = (int32)reverse.size() - 1; i >= 0; --i)
		outPath.push_back(_graph[reverse[i]].position);
	outPath.push_back(goal);
	return true;
}

} // End of namespace ZeroComico
