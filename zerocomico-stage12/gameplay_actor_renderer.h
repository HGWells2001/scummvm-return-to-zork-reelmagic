/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE12_GAMEPLAY_ACTOR_RENDERER_H
#define ZEROCOMICO_STAGE12_GAMEPLAY_ACTOR_RENDERER_H

#include "zerocomico-stage11/integrated_gameplay_host.h"
#include "zerocomico-stage12/actor_render_frame.h"
#include "zerocomico-stage12/actor_render_submitter.h"

namespace ZeroComico {

class GameplayActorRenderer {
public:
	GameplayActorRenderer(const IntegratedGameplayHost &host,
	                      const SceneRegistry &registry);

	bool render(const TransformSample &actorRoot,
	            ActorTriangleSink &sink,
	            Common::String &errorMessage);

	const ActorRenderFrame &lastFrame() const { return _lastFrame; }

private:
	const IntegratedGameplayHost &_host;
	const SceneRegistry &_registry;
	ActorRenderFrame _lastFrame;
};

} // End of namespace ZeroComico

#endif
