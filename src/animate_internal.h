/**
 * @file animate_internal.h
 * @brief Internal structures for PointerPro Animate
 *
 * This header defines the internal data structures used in the implementation.
 * These are not part of the public API.
 */

#ifndef ANIMATE_INTERNAL_H
#define ANIMATE_INTERNAL_H

#include "animate.h"

struct sprite {
    uint16_t width;
    uint16_t height;
    uint32_t *pixels;      // sprite 自己唯一持有的一份像素数据
    size_t ref_count;      // 当前有多少 placement 正在引用它
};

struct sprite_placement {
    struct sprite *sprite; // 指向共享 sprite，不复制像素数据
    int x;
    int y;

    struct canvas *owner;  // 可选，但很适合做一致性检查/解释实现

    struct sprite_placement *prev;
    struct sprite_placement *next;
};

struct canvas {
    uint16_t width;
    uint16_t height;

    struct sprite_placement *front; // 最底层
    struct sprite_placement *back;  // 最顶层

    size_t placement_count;         // 可选，不是 O(1) 的关键，但便于管理
};

#endif /* ANIMATE_INTERNAL_H */
