/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include <cstring>

#include "common/endian.h"

#include "zerocomico-stage9/anj_tracks.h"

namespace ZeroComico {

namespace {

class TrackCursor {
public:
	TrackCursor(const Common::Array<byte> &data, uint32 offset, uint32 size) :
		_data(data), _pos(offset), _end(offset + size) {}

	uint32 pos() const { return _pos; }
	uint32 remaining() const { return _pos <= _end ? _end - _pos : 0; }
	bool eos() const { return _pos == _end; }

	bool readU32(uint32 &v) {
		if (remaining() < 4)
			return false;
		v = READ_LE_UINT32(_data.data() + _pos);
		_pos += 4;
		return true;
	}

	bool readFloat(float &v) {
		if (remaining() < 4)
			return false;
		v = READ_LE_FLOAT32(_data.data() + _pos);
		_pos += 4;
		return true;
	}

	bool readName32(Common::String &name) {
		if (remaining() < 32)
			return false;
		uint32 length = 0;
		while (length < 32 && _data[_pos + length] != 0)
			++length;
		name = Common::String((const char *)_data.data() + _pos, length);
		_pos += 32;
		return true;
	}

private:
	const Common::Array<byte> &_data;
	uint32 _pos;
	uint32 _end;
};

static bool readVec3(TrackCursor &c, Vec3f &v) {
	return c.readFloat(v.x) && c.readFloat(v.y) && c.readFloat(v.z);
}

static bool readVec3Channel(TrackCursor &c,
                            Common::Array<TcbVec3Key> *keys,
                            Common::String &error) {
	uint32 count = 0;
	if (!c.readU32(count) || count > 100000) {
		error = "Invalid ANJ vec3 key count";
		return false;
	}

	if (keys)
		keys->resize(count);

	for (uint32 i = 0; i < count; ++i) {
		uint32 frame = 0;
		TcbVec3Key key;
		if (!c.readU32(frame) ||
		    !c.readFloat(key.tension) ||
		    !c.readFloat(key.continuity) ||
		    !c.readFloat(key.bias) ||
		    !readVec3(c, key.value)) {
			error = "Truncated ANJ vec3 key";
			return false;
		}
		key.frame = (int32)frame;
		if (keys)
			(*keys)[i] = key;
	}
	return true;
}

static bool readScalarChannel(TrackCursor &c, Common::String &error) {
	uint32 count = 0;
	if (!c.readU32(count) || count > 100000) {
		error = "Invalid ANJ scalar key count";
		return false;
	}
	for (uint32 i = 0; i < count; ++i) {
		uint32 frame = 0;
		float tension = 0.0f, continuity = 0.0f, bias = 0.0f, value = 0.0f;
		if (!c.readU32(frame) ||
		    !c.readFloat(tension) ||
		    !c.readFloat(continuity) ||
		    !c.readFloat(bias) ||
		    !c.readFloat(value)) {
			error = "Truncated ANJ scalar key";
			return false;
		}
		(void)frame;
		(void)tension;
		(void)continuity;
		(void)bias;
		(void)value;
	}
	return true;
}

static bool readRotationChannel(TrackCursor &c,
                                Common::Array<TcbAxisAngleKey> *keys,
                                Common::String &error) {
	uint32 count = 0;
	if (!c.readU32(count) || count > 100000) {
		error = "Invalid ANJ rotation key count";
		return false;
	}

	if (keys)
		keys->resize(count);

	for (uint32 i = 0; i < count; ++i) {
		uint32 frame = 0;
		TcbAxisAngleKey key;
		if (!c.readU32(frame) ||
		    !c.readFloat(key.tension) ||
		    !c.readFloat(key.continuity) ||
		    !c.readFloat(key.bias) ||
		    !readVec3(c, key.axis) ||
		    !c.readFloat(key.angle)) {
			error = "Truncated ANJ rotation key";
			return false;
		}
		key.frame = (int32)frame;
		if (keys)
			(*keys)[i] = key;
	}
	return true;
}

static bool readVisibilityChannel(TrackCursor &c,
                                  Common::Array<VisibilityKey> *keys,
                                  Common::String &error) {
	uint32 count = 0;
	if (!c.readU32(count) || count > 100000) {
		error = "Invalid ANJ visibility key count";
		return false;
	}

	if (keys)
		keys->resize(count);

	for (uint32 i = 0; i < count; ++i) {
		uint32 frame = 0;
		uint32 visible = 0;
		if (!c.readU32(frame) || !c.readU32(visible)) {
			error = "Truncated ANJ visibility key";
			return false;
		}
		if (keys) {
			(*keys)[i].frame = (int32)frame;
			(*keys)[i].visible = visible != 0;
		}
	}
	return true;
}

static int32 findMesh(const P3DModel &model, const Common::String &name) {
	for (uint32 i = 0; i < model.meshes.size(); ++i) {
		if (model.meshes[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

static int32 findCamera(const P3DModel &model, const Common::String &name) {
	for (uint32 i = 0; i < model.cameras.size(); ++i) {
		if (model.cameras[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

static int32 findLight(const P3DModel &model, const Common::String &name) {
	for (uint32 i = 0; i < model.lights.size(); ++i) {
		if (model.lights[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

static int32 findBinding(const ANJDocument &document, const Common::String &name) {
	for (uint32 i = 0; i < document.bindings.size(); ++i) {
		if (document.bindings[i].objectName.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

static ANJTargetKind kindFromType(uint16 type) {
	switch (type) {
	case 0xF003:
	case 0xF032:
		return kANJTargetTransform;
	case 0xF011:
		return kANJTargetCameraTarget;
	case 0xF001:
		return kANJTargetCamera;
	case 0xF002:
		return kANJTargetLight;
	case 0xF022:
		return kANJTargetSingleVec3;
	default:
		return kANJTargetUnknown;
	}
}

static ANJTargetKind resolveTarget(const P3DModel &model,
                                   const ANJDocument &document,
                                   const Common::String &name) {
	if (findMesh(model, name) >= 0)
		return kANJTargetTransform;
	if (findCamera(model, name) >= 0)
		return kANJTargetCamera;
	if (findLight(model, name) >= 0)
		return kANJTargetLight;

	if (name.hasSuffixIgnoreCase(".target")) {
		const Common::String cameraName = name.substr(0, name.size() - 7);
		if (findCamera(model, cameraName) >= 0)
			return kANJTargetCameraTarget;
	}

	const int32 binding = findBinding(document, name);
	if (binding >= 0)
		return kindFromType(document.bindings[binding].type);

	return kANJTargetUnknown;
}

static AnimationClip *findOrAddClip(Common::Array<AnimationClip> &clips,
                                    const ANJTimelineHeader &timeline) {
	for (uint32 i = 0; i < clips.size(); ++i) {
		if (clips[i].name.equalsIgnoreCase(timeline.animationName)) {
			if ((int32)timeline.firstFrame < clips[i].firstFrame)
				clips[i].firstFrame = (int32)timeline.firstFrame;
			if ((int32)timeline.lastFrame > clips[i].lastFrame)
				clips[i].lastFrame = (int32)timeline.lastFrame;
			return &clips[i];
		}
	}

	AnimationClip clip;
	clip.name = timeline.animationName;
	clip.firstFrame = (int32)timeline.firstFrame;
	clip.lastFrame = (int32)timeline.lastFrame;
	clip.framesPerSecond = 25.0f;
	clips.push_back(clip);
	return &clips.back();
}

static ObjectTimeline *findOrAddTrack(AnimationClip &clip,
                                      const Common::String &objectName) {
	for (uint32 i = 0; i < clip.tracks.size(); ++i) {
		if (clip.tracks[i].objectName.equalsIgnoreCase(objectName))
			return &clip.tracks[i];
	}

	ObjectTimeline track;
	track.objectName = objectName;
	clip.tracks.push_back(track);
	return &clip.tracks.back();
}

static bool consumeTarget(TrackCursor &cursor,
                          ANJTargetKind kind,
                          const Common::String &targetName,
                          AnimationClip *clip,
                          ANJDecodeStats &stats,
                          Common::String &error) {
	switch (kind) {
	case kANJTargetTransform: {
		ObjectTimeline *track = clip ? findOrAddTrack(*clip, targetName) : nullptr;
		if (!readVec3Channel(cursor, track ? &track->translationKeys : nullptr, error) ||
		    !readVec3Channel(cursor, track ? &track->scaleKeys : nullptr, error) ||
		    !readRotationChannel(cursor, track ? &track->rotationKeys : nullptr, error) ||
		    !readVisibilityChannel(cursor, track ? &track->visibilityKeys : nullptr, error))
			return false;
		++stats.transformTargets;
		return true;
	}

	case kANJTargetCameraTarget:
		if (!readVec3Channel(cursor, nullptr, error))
			return false;
		++stats.cameraTargets;
		return true;

	case kANJTargetCamera:
		if (!readVec3Channel(cursor, nullptr, error) ||
		    !readScalarChannel(cursor, error) ||
		    !readScalarChannel(cursor, error))
			return false;
		++stats.cameras;
		return true;

	case kANJTargetLight:
		if (!readVec3Channel(cursor, nullptr, error) ||
		    !readVec3Channel(cursor, nullptr, error))
			return false;
		++stats.lights;
		return true;

	case kANJTargetSingleVec3:
		if (!readVec3Channel(cursor, nullptr, error))
			return false;
		++stats.singleVec3Targets;
		return true;

	default:
		++stats.unresolvedTargets;
		error = Common::String::format("Unresolved ANJ target '%s'", targetName.c_str());
		return false;
	}
}

} // namespace

ANJDecodeStats::ANJDecodeStats() :
	timelines(0),
	transformTargets(0),
	cameraTargets(0),
	cameras(0),
	lights(0),
	singleVec3Targets(0),
	unresolvedTargets(0) {
}

bool ANJTrackDecoder::decode(const P3DModel &model,
                             const ANJDocument &document,
                             Common::Array<AnimationClip> &clips,
                             ANJDecodeStats &stats,
                             Common::String &errorMessage) const {
	clips.clear();
	stats = ANJDecodeStats();
	errorMessage.clear();

	for (uint32 i = 0; i < document.timelines.size(); ++i) {
		const ANJTimelineHeader &timeline = document.timelines[i];
		if (timeline.payloadOffset > document.decoded.size() ||
		    timeline.payloadSize > document.decoded.size() - timeline.payloadOffset) {
			errorMessage = "ANJ timeline payload is outside the decoded buffer";
			return false;
		}

		AnimationClip *clip = findOrAddClip(clips, timeline);
		TrackCursor cursor(document.decoded, timeline.payloadOffset, timeline.payloadSize);
		++stats.timelines;

		while (!cursor.eos()) {
			Common::String targetName;
			if (!cursor.readName32(targetName) || targetName.empty()) {
				errorMessage = Common::String::format(
					"Timeline '%s' has an invalid target name at +%u",
					timeline.animationName.c_str(), cursor.pos());
				return false;
			}

			const ANJTargetKind kind = resolveTarget(model, document, targetName);
			if (!consumeTarget(cursor, kind, targetName, clip, stats, errorMessage)) {
				errorMessage = Common::String::format(
					"Timeline '%s', target '%s': %s",
					timeline.animationName.c_str(), targetName.c_str(),
					errorMessage.c_str());
				return false;
			}
		}
	}

	return true;
}

} // End of namespace ZeroComico
