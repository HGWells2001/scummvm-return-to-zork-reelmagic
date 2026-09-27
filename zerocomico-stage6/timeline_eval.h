/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#ifndef ZEROCOMICO_STAGE6_TIMELINE_EVAL_H
#define ZEROCOMICO_STAGE6_TIMELINE_EVAL_H

#include "common/array.h"
#include "common/scummsys.h"

namespace ZeroComico {

struct Vec3f {
	float x;
	float y;
	float z;

	Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
	Vec3f(float px, float py, float pz) : x(px), y(py), z(pz) {}

	Vec3f operator+(const Vec3f &b) const { return Vec3f(x + b.x, y + b.y, z + b.z); }
	Vec3f operator-(const Vec3f &b) const { return Vec3f(x - b.x, y - b.y, z - b.z); }
	Vec3f operator*(float s) const { return Vec3f(x * s, y * s, z * s); }
};

struct Quatf {
	float x;
	float y;
	float z;
	float w;

	Quatf() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quatf(float qx, float qy, float qz, float qw) : x(qx), y(qy), z(qz), w(qw) {}
};

struct TcbVec3Key {
	int32 frame;
	float tension;
	float continuity;
	float bias;
	Vec3f value;
};

struct TcbAxisAngleKey {
	int32 frame;
	float tension;
	float continuity;
	float bias;
	Vec3f axis;
	float angle;
};

struct VisibilityKey {
	int32 frame;
	bool visible;
};

struct TransformSample {
	Vec3f translation;
	Vec3f scale;
	Quatf rotation;
	bool visible;

	TransformSample() : translation(), scale(1.0f, 1.0f, 1.0f), rotation(), visible(true) {}
};

Vec3f evaluateTcbVec3(const Common::Array<TcbVec3Key> &keys, float frame,
                      const Vec3f &fallback);

Quatf axisAngleToQuaternion(const Vec3f &axis, float angleRadians);
Quatf slerpQuaternion(const Quatf &a, const Quatf &b, float t);
Quatf evaluateAxisAngle(const Common::Array<TcbAxisAngleKey> &keys, float frame,
                        const Quatf &fallback);

bool evaluateVisibility(const Common::Array<VisibilityKey> &keys, float frame,
                        bool fallback);

} // End of namespace ZeroComico

#endif
