/*
 * animate.c — PointerPro Animate
 *
 * This file is intentionally thin. All implementations are split across:
 *   canvas.c    — canvas lifecycle and frame size
 *   frame.c     — frame generation
 *   sprite.c    — sprite creation/destruction (rectangle, circle, BMP)
 *   placement.c — placement lifecycle and layer management
 *
 * The public API is declared in animate.h.
 * Internal struct definitions live in animate_internal.h.
 */

#include "animate_internal.h"
