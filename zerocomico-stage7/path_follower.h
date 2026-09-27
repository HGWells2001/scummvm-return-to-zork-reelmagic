/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE7_PATH_FOLLOWER_H
#define ZEROCOMICO_STAGE7_PATH_FOLLOWER_H

#include "common/array.h"

#include "zerocomico-stage7/bsp_navigation.h"

namespace ZeroComico {

class PathFollower {
public:
	PathFollower();

	void clear();
	bool begin(const BspNavigation &navigation, const NavVec2 &start,
	           const NavVec2 &goal, float unitsPerSecond);

	void update(uint32 deltaMillis);

	bool active() const { return _active; }
	const NavVec2 &position() const { return _position; }
	const NavVec2 &goal() const { return _goal; }

private:
	Common::Array<NavVec2> _path;
	uint32 _segment;
	NavVec2 _position;
	NavVec2 _goal;
	float _speed;
	bool _active;
};

} // End of namespace ZeroComico

#endif
