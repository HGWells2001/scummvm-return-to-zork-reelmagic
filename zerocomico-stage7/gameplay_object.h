/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE7_GAMEPLAY_OBJECT_H
#define ZEROCOMICO_STAGE7_GAMEPLAY_OBJECT_H

#include "common/array.h"
#include "common/str.h"

namespace ZeroComico {

struct GameplayObject {
	Common::String name;
	Common::String entity;
	Common::String polygon;
	Common::String roomScope;
	Common::String examineText;

	float range;
	float operateRange;
	float size;

	bool pickable;
	bool examinable;
	bool operated;
	bool enabled;
	bool hasExamineHandler;
	bool hasOperateHandler;

	GameplayObject();
};

class GameplayObjectRegistry {
public:
	void clear();
	bool parse(const Common::String &decodedPuzzleScript, Common::String &errorMessage);

	int32 findByName(const Common::String &name) const;
	int32 findByEntity(const Common::String &entity) const;
	int32 resolvePickedEntity(const Common::String &sceneEntity) const;

	const GameplayObject *object(int32 id) const;
	uint32 size() const { return _objects.size(); }

private:
	Common::Array<GameplayObject> _objects;
};

} // End of namespace ZeroComico

#endif
