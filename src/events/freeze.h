#ifndef RE_EVENTS_FREEZE_H
#define RE_EVENTS_FREEZE_H

// Freeze Mario in position and animation for 1-5 seconds.

#define FREEZE_MIN 60
#define FREEZE_MAX 300

static RE_Timer sFreezeTimer = {0};
static f32 sFreezeX, sFreezeY, sFreezeZ;

static void do_freeze(struct MarioState *m) {
    re_timer_set(&sFreezeTimer, rng_range(FREEZE_MIN, FREEZE_MAX));
    sFreezeX = m->pos[0];
    sFreezeY = m->pos[1];
    sFreezeZ = m->pos[2];
}

static void tick_freeze(struct MarioState *m) {
    if (!re_timer_active(&sFreezeTimer)) return;

    m->pos[0]      = sFreezeX;
    m->pos[1]      = sFreezeY;
    m->pos[2]      = sFreezeZ;
    m->vel[0]      = 0.0f;
    m->vel[1]      = 0.0f;
    m->vel[2]      = 0.0f;
    m->forwardVel  = 0.0f;
    m->intendedMag = 0.0f;

    // Freeze animation in place.
    if (m->marioObj) {
        m->marioObj->header.gfx.animInfo.animAccel = 0;
        m->marioObj->header.gfx.animInfo.animFrame = 0;
    }

    re_timer_tick(&sFreezeTimer);
}

#endif // RE_EVENTS_FREEZE_H
