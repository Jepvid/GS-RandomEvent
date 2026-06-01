#ifndef RE_EVENTS_WIND_H
#define RE_EVENTS_WIND_H

#include "engine/math_util.h"
#include <math.h>

#define WIND_DURATION 300
#define WIND_ACCEL    3.0f
#define WIND_VEL_CAP  60.0f

static int sWindFrames = 0;
static f32 sWindDirX   = 0.0f;
static f32 sWindDirZ   = 0.0f;

static void do_wind(struct MarioState *m) {
    (void)m;
    u16 angle  = (u16)rng_range(0, 65535);
    sWindDirX  = sins(angle);
    sWindDirZ  = coss(angle);
    sWindFrames = WIND_DURATION;
}

static void tick_wind(struct MarioState *m) {
    if (sWindFrames <= 0) return;

    m->vel[0] += sWindDirX * WIND_ACCEL;
    m->vel[2] += sWindDirZ * WIND_ACCEL;

    f32 wx = m->vel[0];
    f32 wz = m->vel[2];
    f32 spd = wx * wx + wz * wz;
    if (spd > WIND_VEL_CAP * WIND_VEL_CAP) {
        f32 scale = WIND_VEL_CAP / sqrtf(spd);
        m->vel[0] = wx * scale;
        m->vel[2] = wz * scale;
    }

    // Keep Mario in a moving action so physics actually applies the velocity.
    if (!(m->action & ACT_FLAG_AIR) && m->action != ACT_WALKING && m->action != ACT_RUNNING)
        set_mario_action(m, ACT_WALKING, 0);

    sWindFrames--;
}

#endif // RE_EVENTS_WIND_H
