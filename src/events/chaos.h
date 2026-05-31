#ifndef RE_EVENTS_CHAOS_H
#define RE_EVENTS_CHAOS_H

// Requires: sm64.h (MarioState, SURFACE_ICE, ACT_FLAG_AIR)
#include "surface_terrains.h"

#define CHAOS_DURATION 600  // 20 seconds at 30fps

// ---- Gravity ----

static int sGravityFrames = 0;
static f32 sGravityBias   = 0.0f;

static void do_gravity_light(struct MarioState *m) {
    (void)m;
    sGravityBias   = 2.0f;   // add upward force each frame (half gravity)
    sGravityFrames = CHAOS_DURATION;
}

static void do_gravity_heavy(struct MarioState *m) {
    (void)m;
    sGravityBias   = -2.0f;  // add downward force each frame (1.5x gravity)
    sGravityFrames = CHAOS_DURATION;
}

// ---- Slippery floor ----

static int sSlippery = 0;

static void do_slippery(struct MarioState *m) {
    (void)m;
    sSlippery = CHAOS_DURATION;
}

// ---- Tick — called every game frame ----

static void tick_chaos(struct MarioState *m) {
    if (sGravityFrames > 0) {
        // Bias is only meaningful when airborne.
        if (m->action & ACT_FLAG_AIR)
            m->vel[1] += sGravityBias;
        sGravityFrames--;
    }

    if (sSlippery > 0) {
        // Overwrite the current floor's terrain type every frame.
        // Static surfaces persist this change; dynamic ones are rebuilt anyway.
        if (m->floor)
            m->floor->type = SURFACE_ICE;
        sSlippery--;
    }
}

#endif // RE_EVENTS_CHAOS_H
