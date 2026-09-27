/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#ifndef ZEROCOMICO_STAGE6_MENU_HIGHLIGHT_H
#define ZEROCOMICO_STAGE6_MENU_HIGHLIGHT_H

#include "common/str.h"

namespace ZeroComico {

/**
 * The retail menu stores paired button textures as *_spe.tga (spento) and
 * *_acc.tga (acceso).  These helpers derive the partner resource name without
 * hard-coding NUOVO/CARICA/SALVA/etc.
 */
bool isMenuButtonTexture(const Common::String &textureName);
Common::String menuButtonLitTexture(const Common::String &textureName);
Common::String menuButtonUnlitTexture(const Common::String &textureName);

} // End of namespace ZeroComico

#endif
