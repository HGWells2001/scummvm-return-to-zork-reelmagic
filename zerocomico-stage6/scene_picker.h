/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_SCENE_PICKER_H
#define ZEROCOMICO_STAGE6_SCENE_PICKER_H

#include "common/array.h"
#include "common/scummsys.h"

namespace ZeroComico {

/**
 * Software ID buffer paired with the Stage 5 depth buffer.
 *
 * The renderer writes the stable mesh id whenever a fragment wins the
 * depth test. Input code can then resolve a mouse pixel to the exact visible
 * mesh without a second ray/triangle pass and without bounding-box guesses.
 */
class ScenePicker {
public:
	ScenePicker();

	void resize(uint16 width, uint16 height);
	void clear();

	uint16 width() const { return _width; }
	uint16 height() const { return _height; }

	/**
	 * Submit a fragment after perspective projection.
	 *
	 * @param x       framebuffer x
	 * @param y       framebuffer y
	 * @param depth   same monotonically comparable depth used by renderer
	 * @param meshId  stable scene-registry id, or -1 for non-pickable
	 * @return true if this fragment replaced the previous visible fragment
	 */
	bool writePixel(int x, int y, float depth, int32 meshId);

	/**
	 * Return the visible mesh id at a framebuffer pixel, or -1.
	 */
	int32 pick(int x, int y) const;

	/**
	 * Depth at a pixel. Large positive value means "nothing rendered".
	 */
	float depthAt(int x, int y) const;

private:
	bool valid(int x, int y) const;
	uint32 offset(int x, int y) const;

	uint16 _width;
	uint16 _height;
	Common::Array<float> _depth;
	Common::Array<int32> _ids;
};

} // End of namespace ZeroComico

#endif
