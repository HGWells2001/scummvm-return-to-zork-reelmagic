/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/scene_picker.h"

#include <cfloat>

namespace ZeroComico {

ScenePicker::ScenePicker() : _width(0), _height(0) {
}

void ScenePicker::resize(uint16 width, uint16 height) {
	_width = width;
	_height = height;

	const uint32 count = (uint32)_width * (uint32)_height;
	_depth.resize(count);
	_ids.resize(count);
	clear();
}

void ScenePicker::clear() {
	const uint32 count = (uint32)_width * (uint32)_height;
	for (uint32 i = 0; i < count; ++i) {
		_depth[i] = FLT_MAX;
		_ids[i] = -1;
	}
}

bool ScenePicker::valid(int x, int y) const {
	return x >= 0 && y >= 0 && x < _width && y < _height;
}

uint32 ScenePicker::offset(int x, int y) const {
	return (uint32)y * (uint32)_width + (uint32)x;
}

bool ScenePicker::writePixel(int x, int y, float depth, int32 meshId) {
	if (!valid(x, y))
		return false;

	const uint32 i = offset(x, y);
	if (depth >= _depth[i])
		return false;

	_depth[i] = depth;
	_ids[i] = meshId;
	return true;
}

int32 ScenePicker::pick(int x, int y) const {
	if (!valid(x, y))
		return -1;
	return _ids[offset(x, y)];
}

float ScenePicker::depthAt(int x, int y) const {
	if (!valid(x, y))
		return FLT_MAX;
	return _depth[offset(x, y)];
}

} // End of namespace ZeroComico
