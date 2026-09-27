/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE7_BSP_NAVIGATION_H
#define ZEROCOMICO_STAGE7_BSP_NAVIGATION_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

struct NavVec2 {
	float x;
	float y;

	NavVec2(float px = 0.0f, float py = 0.0f) : x(px), y(py) {}
};

struct BspEdge {
	int32 point0;
	int32 point1;
	int32 frontCell;
	int32 backCell;

	BspEdge() : point0(-1), point1(-1), frontCell(-1), backCell(-1) {}
};

struct BspCell {
	int32 index;
	Common::Array<int32> edgeIndices;

	BspCell() : index(-1) {}
};

struct PathArc {
	int32 node;
	float weight;

	PathArc(int32 n = -1, float w = 0.0f) : node(n), weight(w) {}
};

struct PathNode {
	NavVec2 position;
	Common::Array<PathArc> arcs;
};

/**
 * Parser/runtime for Zero Comico's text .bsp files.
 *
 * These files describe the walkable 2D floor plan and a weighted path graph.
 * They are not rendering BSPs.
 */
class BspNavigation {
public:
	BspNavigation();

	void clear();
	bool parse(const Common::String &text, Common::String &errorMessage);
	bool validate(Common::String &errorMessage) const;

	int32 nearestGraphNode(const NavVec2 &point) const;
	bool findPath(const NavVec2 &start, const NavVec2 &goal,
	              Common::Array<NavVec2> &outPath) const;

	const Common::Array<NavVec2> &outline() const { return _outline; }
	const Common::Array<Common::Array<NavVec2> > &holes() const { return _holes; }
	const Common::Array<NavVec2> &points() const { return _points; }
	const Common::Array<BspEdge> &edges() const { return _edges; }
	const Common::Array<BspCell> &cells() const { return _cells; }
	const Common::Array<PathNode> &graph() const { return _graph; }

private:
	Common::Array<NavVec2> _outline;
	Common::Array<Common::Array<NavVec2> > _holes;
	Common::Array<NavVec2> _points;
	Common::Array<BspEdge> _edges;
	Common::Array<BspCell> _cells;
	Common::Array<PathNode> _graph;
	Common::Array<int32> _treeLeaves;
	int32 _treeNodes;
	int32 _treeNulls;
};

} // End of namespace ZeroComico

#endif
