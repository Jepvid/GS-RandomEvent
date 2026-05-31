#ifndef RE_EVENTS_MAGNETISM_H
#define RE_EVENTS_MAGNETISM_H

#include "game/object_list_processor.h"

#define MAGNET_DURATION  600
#define MAGNET_RADIUS   1500.0f
#define MAGNET_SPEED       5.0f

static int sMagnetFrames = 0;

static void do_magnetism(struct MarioState *m) {
    (void)m;
    sMagnetFrames = MAGNET_DURATION;
}

static void tick_magnetism(struct MarioState *m) {
    if (sMagnetFrames <= 0) return;

    for (int i = 0; i < OBJECT_POOL_CAPACITY; i++) {
        struct Object *o = &gObjectPool[i];
        if (!(o->oFlags & ACTIVE_FLAG_ACTIVE)) continue;
        if (o == m->marioObj) continue;

        f32 dx  = m->pos[0] - o->oPosX;
        f32 dy  = m->pos[1] - o->oPosY;
        f32 dz  = m->pos[2] - o->oPosZ;
        f32 adx = dx < 0.0f ? -dx : dx;
        f32 ady = dy < 0.0f ? -dy : dy;
        f32 adz = dz < 0.0f ? -dz : dz;
        // Chebyshev distance avoids sqrtf
        f32 dist = adx > ady ? (adx > adz ? adx : adz) : (ady > adz ? ady : adz);

        if (dist > MAGNET_RADIUS || dist < 1.0f) continue;

        f32 inv  = MAGNET_SPEED / dist;
        o->oPosX += dx * inv;
        o->oPosY += dy * inv;
        o->oPosZ += dz * inv;
    }

    sMagnetFrames--;
}

#endif // RE_EVENTS_MAGNETISM_H
