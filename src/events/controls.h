#ifndef RE_EVENTS_CONTROLS_H
#define RE_EVENTS_CONTROLS_H

#define CONTROLS_DURATION 600

#define SLOW_FWD_CAP   24.0f
#define SLOW_JUMP_CAP  21.0f
#define FAST_FWD_CAP   96.0f
#define FAST_JUMP_CAP  84.0f

static int sRevFrames  = 0;
static int sSlowFrames = 0;
static int sFastFrames = 0;
static int sFastWasAir = 0;

static void do_rev_controls(struct MarioState *m) {
    (void)m;
    sRevFrames = CONTROLS_DURATION;
}

static void do_slowmo(struct MarioState *m) {
    (void)m;
    sFastFrames = 0;
    sSlowFrames = CONTROLS_DURATION;
}

static void do_highspeed(struct MarioState *m) {
    sSlowFrames = 0;
    sFastFrames = CONTROLS_DURATION;
    sFastWasAir = (m->action & ACT_FLAG_AIR) != 0;
}

static void tick_controls(struct MarioState *m) {
    if (sRevFrames > 0) {
        m->intendedYaw += 0x8000;
        sRevFrames--;
    }

    if (sSlowFrames > 0) {
        m->intendedMag *= 0.5f;
        if (m->forwardVel >  SLOW_FWD_CAP) mario_set_forward_vel(m,  SLOW_FWD_CAP);
        if (m->forwardVel < -SLOW_FWD_CAP) mario_set_forward_vel(m, -SLOW_FWD_CAP);
        if (m->vel[1] > SLOW_JUMP_CAP)     m->vel[1] = SLOW_JUMP_CAP;
        if (m->marioObj)
            m->marioObj->header.gfx.animInfo.animAccel = 0x8000;
        sSlowFrames--;
        if (sSlowFrames == 0 && m->marioObj)
            m->marioObj->header.gfx.animInfo.animAccel = 0;
    }

    if (sFastFrames > 0) {
        if (m->intendedMag > 0.5f) m->intendedMag = 32.0f;
        if (m->forwardVel > 2.0f && m->forwardVel < FAST_FWD_CAP) {
            f32 nv = m->forwardVel + 8.0f;
            mario_set_forward_vel(m, nv > FAST_FWD_CAP ? FAST_FWD_CAP : nv);
        }
        // Double vel[1] once on the ground→air transition
        int nowAir = (m->action & ACT_FLAG_AIR) != 0;
        if (nowAir && !sFastWasAir && m->vel[1] > 0.0f) {
            m->vel[1] *= 2.0f;
            if (m->vel[1] > FAST_JUMP_CAP) m->vel[1] = FAST_JUMP_CAP;
        }
        sFastWasAir = nowAir;
        if (m->marioObj)
            m->marioObj->header.gfx.animInfo.animAccel = 0x20000;
        sFastFrames--;
        if (sFastFrames == 0 && m->marioObj)
            m->marioObj->header.gfx.animInfo.animAccel = 0;
    }
}

#endif // RE_EVENTS_CONTROLS_H
