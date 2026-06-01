#ifndef RE_EVENTS_MOVEMENT_H
#define RE_EVENTS_MOVEMENT_H

#include "audio/external.h"

static int sNarcolepsy   = 0;
static int sNuhUhFrames  = 0;
static int sRunFrames    = 0;
static int sFireFrames   = 0;

static void do_narcolepsy(struct MarioState *m) {
    (void)m;
    sNarcolepsy = 90;
}

static void do_nuh_uh(struct MarioState *m) {
    (void)m;
    sNuhUhFrames = 150;
}

// Cannon launch in random direction with random power.
static void do_launch_up(struct MarioState *m) {
    u16 angle = (u16)rng_range(0, 65535);
    f32 power = (f32)rng_range(40, 100);
    set_mario_action(m, ACT_FREEFALL, 0);
    m->faceAngle[1] = angle;
    mario_set_forward_vel(m, power);
    m->vel[1] = (f32)rng_range(50, 90);
}

static void do_squish(struct MarioState *m) {
    m->squishTimer = 120;
}

static void do_speed_reverse(struct MarioState *m) {
    mario_set_forward_vel(m, -50.0f);
}

static void do_fire(struct MarioState *m) {
    play_sound(SOUND_MARIO_ON_FIRE, m->marioObj->header.gfx.cameraToObject);
    set_mario_action(m, ACT_BURNING_FALL, 0);
    m->vel[1] = 60.0f;
    sFireFrames = 0;
}

// "Cant stop running" — locks Mario into a run at high speed for 10 seconds.
static void do_speed_boost(struct MarioState *m) {
    (void)m;
    sRunFrames = 300;
}

static void tick_movement_states(struct MarioState *m) {
    if (sNarcolepsy > 0) {
        if (sNarcolepsy == 90 && !(m->action & ACT_FLAG_AIR))
            set_mario_action(m, ACT_SLEEPING, 0);
        // Pin position and block all input so mario cant run away.
        m->vel[0]      = 0.0f;
        m->vel[1]      = 0.0f;
        m->vel[2]      = 0.0f;
        m->forwardVel  = 0.0f;
        m->input       = 0;
        m->intendedMag = 0.0f;
        sNarcolepsy--;
    }

    if (sNuhUhFrames > 0) {
        if (m->action & ACT_FLAG_ATTACKING)
            set_mario_action(m, ACT_IDLE, 0);
        sNuhUhFrames--;
    }

    if (sRunFrames > 0) {
        // Force Mario into a run if he's on the ground.
        if (!(m->action & ACT_FLAG_AIR) && !(m->action & ACT_FLAG_SWIMMING)) {
            if (m->action != ACT_WALKING)
                set_mario_action(m, ACT_WALKING, 0);
            mario_set_forward_vel(m, 48.0f);
            m->intendedYaw = m->faceAngle[1];
            m->intendedMag = 32.0f;
        }
        sRunFrames--;
    }
}

#endif // RE_EVENTS_MOVEMENT_H
