/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage7/path_follower.h"

#include <cmath>

namespace ZeroComico {

PathFollower::PathFollower() :
	_segment(0),
	_speed(0.0f),
	_active(false) {
}

void PathFollower::clear() {
	_path.clear();
	_segment = 0;
	_speed = 0.0f;
	_active = false;
}

bool PathFollower::begin(const BspNavigation &navigation, const NavVec2 &start,
                         const NavVec2 &goal, float unitsPerSecond) {
	clear();
	if (unitsPerSecond <= 0.0f)
		return false;
	if (!navigation.findPath(start, goal, _path) || _path.size() < 2)
		return false;

	_position = start;
	_goal = goal;
	_speed = unitsPerSecond;
	_segment = 1;
	_active = true;
	return true;
}

void PathFollower::update(uint32 deltaMillis) {
	if (!_active || _segment >= _path.size())
		return;

	float remaining = _speed * ((float)deltaMillis / 1000.0f);
	while (_active && remaining > 0.0f && _segment < _path.size()) {
		const NavVec2 target = _path[_segment];
		const float dx = target.x - _position.x;
		const float dy = target.y - _position.y;
		const float distance = std::sqrt(dx * dx + dy * dy);

		if (distance <= 0.0001f) {
			_position = target;
			++_segment;
			if (_segment >= _path.size())
				_active = false;
			continue;
		}

		if (remaining >= distance) {
			_position = target;
			remaining -= distance;
			++_segment;
			if (_segment >= _path.size())
				_active = false;
		} else {
			const float scale = remaining / distance;
			_position.x += dx * scale;
			_position.y += dy * scale;
			remaining = 0.0f;
		}
	}

	if (!_active)
		_position = _goal;
}

} // End of namespace ZeroComico
