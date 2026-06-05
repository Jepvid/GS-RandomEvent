#ifndef RE_EVENTS_CHAOS_H
#define RE_EVENTS_CHAOS_H

#include "surface_terrains.h"

#define CHAOS_DURATION 600 // 20 seconds

static RE_Timer sGravityFrames = {0};
static f32      sGravityBias   = 0.0f;
static RE_Timer sSlippery      = {0};

static void do_gravity_light(struct MarioState *m) {
    (void)m;
    sGravityBias   =  2.0f;
    re_timer_set(&sGravityFrames, CHAOS_DURATION);
}

static void do_gravity_heavy(struct MarioState *m) {
    (void)m;
    sGravityBias   = -2.0f;
    re_timer_set(&sGravityFrames, CHAOS_DURATION);
}

static void do_slippery(struct MarioState *m) {
    (void)m;
    re_timer_set(&sSlippery, CHAOS_DURATION);
}

static void tick_chaos(struct MarioState *m) {
    if (re_timer_active(&sGravityFrames)) {
        if (m->action & ACT_FLAG_AIR)
            m->vel[1] += sGravityBias;
        re_timer_tick(&sGravityFrames);
    }

    if (re_timer_active(&sSlippery)) {
        if (m->floor)
            m->floor->type = SURFACE_ICE;
        re_timer_tick(&sSlippery);
    }
}

#endif // RE_EVENTS_CHAOS_H
