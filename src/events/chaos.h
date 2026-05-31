#ifndef RE_EVENTS_CHAOS_H
#define RE_EVENTS_CHAOS_H

#include "surface_terrains.h"

#define CHAOS_DURATION 600 // 20 seconds

static int sGravityFrames = 0;
static f32 sGravityBias   = 0.0f;
static int sSlippery      = 0;

static void do_gravity_light(struct MarioState *m) {
    (void)m;
    sGravityBias   =  2.0f; // +2/frame while airborne → roughly half gravity
    sGravityFrames = CHAOS_DURATION;
}

static void do_gravity_heavy(struct MarioState *m) {
    (void)m;
    sGravityBias   = -2.0f; // -2/frame while airborne → roughly 1.5x gravity
    sGravityFrames = CHAOS_DURATION;
}

static void do_slippery(struct MarioState *m) {
    (void)m;
    sSlippery = CHAOS_DURATION;
}

static void tick_chaos(struct MarioState *m) {
    if (sGravityFrames > 0) {
        if (m->action & ACT_FLAG_AIR)
            m->vel[1] += sGravityBias;
        sGravityFrames--;
    }

    if (sSlippery > 0) {
        if (m->floor)
            m->floor->type = SURFACE_ICE;
        sSlippery--;
    }
}

#endif // RE_EVENTS_CHAOS_H
