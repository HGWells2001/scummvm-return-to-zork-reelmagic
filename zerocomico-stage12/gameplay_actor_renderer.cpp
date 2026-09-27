/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage12/gameplay_actor_renderer.h"

namespace ZeroComico {

GameplayActorRenderer::GameplayActorRenderer(
		const IntegratedGameplayHost &host,
		const SceneRegistry &registry) :
	_host(host),
	_registry(registry) {
}

bool GameplayActorRenderer::render(const TransformSample &actorRoot,
                                   ActorTriangleSink &sink,
                                   Common::String &errorMessage) {
	ActorRenderFrameBuilder builder;
	if (!builder.build(_host.actor(), _host.renderCatalog(), _registry,
	                   actorRoot, _lastFrame, errorMessage))
		return false;

	ActorRenderSubmitter submitter;
	return submitter.submit(_host.actor(), _host.actorTextures(), _lastFrame,
	                        sink, errorMessage);
}

} // End of namespace ZeroComico
