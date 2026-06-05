#ifndef RE_EVENTS_MOVEMENT_H
#define RE_EVENTS_MOVEMENT_H

#include "audio/external.h"

static RE_Timer sNarcolepsy  = {0};
static RE_Timer sNuhUhFrames = {0};
static RE_Timer sRunFrames   = {0};

static void do_narcolepsy(struct MarioState *m) {
    (void)m;
    re_timer_set(&sNarcolepsy, 90);
}

static void do_nuh_uh(struct MarioState *m) {
    (void)m;
    re_timer_set(&sNuhUhFrames, 150);
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
}

// "Cant stop running" — locks Mario into a run at high speed for 10 seconds.
static void do_speed_boost(struct MarioState *m) {
    (void)m;
    re_timer_set(&sRunFrames, 300);
}

static void tick_movement_states(struct MarioState *m) {
    if (re_timer_active(&sNarcolepsy)) {
        if (sNarcolepsy.remaining == 90 && !(m->action & ACT_FLAG_AIR))
            set_mario_action(m, ACT_SLEEPING, 0);
        m->vel[0]      = 0.0f;
        m->vel[1]      = 0.0f;
        m->vel[2]      = 0.0f;
        m->forwardVel  = 0.0f;
        m->intendedMag = 0.0f;
        re_timer_tick(&sNarcolepsy);
    }

    if (re_timer_active(&sNuhUhFrames)) {
        if (m->action & ACT_FLAG_ATTACKING)
            set_mario_action(m, ACT_IDLE, 0);
        re_timer_tick(&sNuhUhFrames);
    }

    if (re_timer_active(&sRunFrames)) {
        if (!(m->action & ACT_FLAG_AIR) && !(m->action & ACT_FLAG_SWIMMING)) {
            if (m->action != ACT_WALKING)
                set_mario_action(m, ACT_WALKING, 0);
            mario_set_forward_vel(m, 48.0f);
            m->intendedYaw = m->faceAngle[1];
            m->intendedMag = 32.0f;
        }
        re_timer_tick(&sRunFrames);
    }
}

#endif // RE_EVENTS_MOVEMENT_H
