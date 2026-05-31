#ifndef RE_EVENTS_WIND_H
#define RE_EVENTS_WIND_H

#include "engine/math_util.h"

#define WIND_DURATION 300
#define WIND_FORCE    2.5f
#define WIND_VEL_CAP  80.0f

static int sWindFrames = 0;
static f32 sWindX      = 0.0f;
static f32 sWindZ      = 0.0f;

static void do_wind(struct MarioState *m) {
    (void)m;
    u16 angle   = (u16)rng_range(0, 65535);
    sWindX      = sins(angle) * WIND_FORCE;
    sWindZ      = coss(angle) * WIND_FORCE;
    sWindFrames = WIND_DURATION;
}

static void tick_wind(struct MarioState *m) {
    if (sWindFrames <= 0) return;
    m->vel[0] += sWindX;
    m->vel[2] += sWindZ;
    if (m->vel[0] >  WIND_VEL_CAP) m->vel[0] =  WIND_VEL_CAP;
    if (m->vel[0] < -WIND_VEL_CAP) m->vel[0] = -WIND_VEL_CAP;
    if (m->vel[2] >  WIND_VEL_CAP) m->vel[2] =  WIND_VEL_CAP;
    if (m->vel[2] < -WIND_VEL_CAP) m->vel[2] = -WIND_VEL_CAP;
    sWindFrames--;
}

#endif // RE_EVENTS_WIND_H
