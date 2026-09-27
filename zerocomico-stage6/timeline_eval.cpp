/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 3 or, at your option, any later version.
 */

#include "zerocomico-stage6/timeline_eval.h"

#include <cmath>

namespace ZeroComico {

static float clamp01(float x) {
	if (x < 0.0f)
		return 0.0f;
	if (x > 1.0f)
		return 1.0f;
	return x;
}

static Vec3f tangentOut(const TcbVec3Key &prev, const TcbVec3Key &cur, const TcbVec3Key &next) {
	const float a = 0.5f * (1.0f - cur.tension) * (1.0f + cur.continuity) * (1.0f + cur.bias);
	const float b = 0.5f * (1.0f - cur.tension) * (1.0f - cur.continuity) * (1.0f - cur.bias);
	return (cur.value - prev.value) * a + (next.value - cur.value) * b;
}

static Vec3f tangentIn(const TcbVec3Key &prev, const TcbVec3Key &cur, const TcbVec3Key &next) {
	const float a = 0.5f * (1.0f - cur.tension) * (1.0f - cur.continuity) * (1.0f + cur.bias);
	const float b = 0.5f * (1.0f - cur.tension) * (1.0f + cur.continuity) * (1.0f - cur.bias);
	return (cur.value - prev.value) * a + (next.value - cur.value) * b;
}

Vec3f evaluateTcbVec3(const Common::Array<TcbVec3Key> &keys, float frame,
                      const Vec3f &fallback) {
	if (keys.empty())
		return fallback;
	if (keys.size() == 1 || frame <= keys[0].frame)
		return keys[0].value;
	if (frame >= keys.back().frame)
		return keys.back().value;

	uint32 i = 0;
	while (i + 1 < keys.size() && frame > keys[i + 1].frame)
		++i;

	const uint32 j = i + 1;
	const TcbVec3Key &k0 = keys[i];
	const TcbVec3Key &k1 = keys[j];
	const TcbVec3Key &km1 = keys[i > 0 ? i - 1 : i];
	const TcbVec3Key &kp2 = keys[j + 1 < keys.size() ? j + 1 : j];

	const float span = (float)(k1.frame - k0.frame);
	if (span <= 0.0f)
		return k1.value;

	const float t = clamp01((frame - k0.frame) / span);
	const float t2 = t * t;
	const float t3 = t2 * t;

	const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
	const float h10 = t3 - 2.0f * t2 + t;
	const float h01 = -2.0f * t3 + 3.0f * t2;
	const float h11 = t3 - t2;

	const Vec3f m0 = tangentOut(km1, k0, k1);
	const Vec3f m1 = tangentIn(k0, k1, kp2);

	return k0.value * h00 + m0 * h10 + k1.value * h01 + m1 * h11;
}

static Quatf normalizeQuaternion(const Quatf &q) {
	const float n = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (n <= 0.000001f)
		return Quatf();
	const float inv = 1.0f / n;
	return Quatf(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
}

Quatf axisAngleToQuaternion(const Vec3f &axis, float angleRadians) {
	const float len = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
	if (len <= 0.000001f)
		return Quatf();

	const float inv = 1.0f / len;
	const float half = angleRadians * 0.5f;
	const float s = std::sin(half);
	return normalizeQuaternion(Quatf(axis.x * inv * s,
	                                 axis.y * inv * s,
	                                 axis.z * inv * s,
	                                 std::cos(half)));
}

Quatf slerpQuaternion(const Quatf &qa, const Quatf &qb, float t) {
	Quatf a = normalizeQuaternion(qa);
	Quatf b = normalizeQuaternion(qb);
	float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	if (dot < 0.0f) {
		dot = -dot;
		b.x = -b.x;
		b.y = -b.y;
		b.z = -b.z;
		b.w = -b.w;
	}

	t = clamp01(t);

	if (dot > 0.9995f) {
		return normalizeQuaternion(Quatf(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t,
			a.w + (b.w - a.w) * t));
	}

	if (dot > 1.0f)
		dot = 1.0f;

	const float theta = std::acos(dot);
	const float sinTheta = std::sin(theta);
	if (std::fabs(sinTheta) <= 0.000001f)
		return a;

	const float wa = std::sin((1.0f - t) * theta) / sinTheta;
	const float wb = std::sin(t * theta) / sinTheta;

	return normalizeQuaternion(Quatf(
		a.x * wa + b.x * wb,
		a.y * wa + b.y * wb,
		a.z * wa + b.z * wb,
		a.w * wa + b.w * wb));
}

Quatf evaluateAxisAngle(const Common::Array<TcbAxisAngleKey> &keys, float frame,
                        const Quatf &fallback) {
	if (keys.empty())
		return fallback;
	if (keys.size() == 1 || frame <= keys[0].frame)
		return axisAngleToQuaternion(keys[0].axis, keys[0].angle);
	if (frame >= keys.back().frame)
		return axisAngleToQuaternion(keys.back().axis, keys.back().angle);

	uint32 i = 0;
	while (i + 1 < keys.size() && frame > keys[i + 1].frame)
		++i;

	const TcbAxisAngleKey &a = keys[i];
	const TcbAxisAngleKey &b = keys[i + 1];
	const float span = (float)(b.frame - a.frame);
	const float t = span > 0.0f ? (frame - a.frame) / span : 1.0f;

	// Stage 6 intentionally uses slerp for rotational keys.  Translation and
	// scale already use the decoded T/C/B parameters.  Reproducing the
	// original engine's quaternion TCB/squad tangent construction is tracked
	// separately and can replace this without changing the public API.
	return slerpQuaternion(axisAngleToQuaternion(a.axis, a.angle),
	                       axisAngleToQuaternion(b.axis, b.angle), t);
}

bool evaluateVisibility(const Common::Array<VisibilityKey> &keys, float frame,
                        bool fallback) {
	if (keys.empty())
		return fallback;

	bool value = fallback;
	for (uint32 i = 0; i < keys.size(); ++i) {
		if (frame < keys[i].frame)
			break;
		value = keys[i].visible;
	}
	return value;
}

} // End of namespace ZeroComico
