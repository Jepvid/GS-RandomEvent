#ifndef RE_EVENTS_BOUNCE_H
#define RE_EVENTS_BOUNCE_H

#include "engine/math_util.h"

// BOUNCY: Mario becomes a rubber ball for 10 seconds.
// Every landing: hard upward bounce, horizontal momentum preserved, no fall damage.

#define BOUNCE_DURATION  300
#define BOUNCE_VEL_MIN    80   // upward velocity on each bounce
#define BOUNCE_VEL_MAX   140
#define BOUNCE_SPD_MIN    20.0f // minimum horizontal speed to keep rolling

static RE_Timer   sBounceTimer = {0};
static ListenerID sBounceListenerID;
static ListenerID sBounceWallID;

static void do_bounce_mode(struct MarioState *m) {
    re_timer_set(&sBounceTimer, BOUNCE_DURATION);

    // If Mario is on the ground, cannon-launch him to start the bounce chain.
    if (m->action & ACT_FLAG_AIR) return;
    m->faceAngle[1] = (u16)rng_range(0, 65535);
    set_mario_action(m, ACT_SHOT_FROM_CANNON, 0);
    m->vel[1] = (f32)rng_range(BOUNCE_VEL_MIN, BOUNCE_VEL_MAX);
    mario_set_forward_vel(m, (f32)rng_range(60, 110));
}

static void on_player_landed(IEvent *event) {
    if (!re_timer_active(&sBounceTimer)) return;
    PlayerLanded *e = (PlayerLanded *)event;
    struct MarioState *m = e->m;

    m->hurtCounter = 0;

    // Preserve horizontal momentum; guarantee minimum so Mario keeps rolling.
    f32 spd = m->forwardVel;
    if (spd > -BOUNCE_SPD_MIN && spd < BOUNCE_SPD_MIN)
        spd = BOUNCE_SPD_MIN;

    set_mario_action(m, ACT_FREEFALL, 0);
    m->vel[1] = (f32)rng_range(BOUNCE_VEL_MIN, BOUNCE_VEL_MAX);
    mario_set_forward_vel(m, spd);
}

// Intercept wall bonks: cancel the bonk, flip direction, bounce back.
static void on_wall_bonk(IEvent *event) {
    if (!re_timer_active(&sBounceTimer)) return;
    PlayerSetAction *e = (PlayerSetAction *)event;
    if (e->action != ACT_SOFT_BONK) return;

    e->Event.Cancelled = true; // block the bonk action

    struct MarioState *m = e->m;

    if (m->wall) {
        // Reflect incoming direction off the wall normal: r = d - 2(d·n)n
        f32 nx  = m->wall->normal.x;
        f32 nz  = m->wall->normal.z;
        f32 dx  = sins(m->faceAngle[1]);
        f32 dz  = coss(m->faceAngle[1]);
        f32 dot = dx * nx + dz * nz;
        m->faceAngle[1] = atan2s(dz - 2.0f * dot * nz, dx - 2.0f * dot * nx);
    } else {
        m->faceAngle[1] += 0x8000; // fallback if wall info unavailable
    }

    f32 spd = m->forwardVel;
    if (spd > -BOUNCE_SPD_MIN && spd < BOUNCE_SPD_MIN)
        spd = BOUNCE_SPD_MIN;

    if (m->vel[1] < 20.0f) m->vel[1] = 20.0f; // keep airborne if falling
    mario_set_forward_vel(m, spd);
    set_mario_action(m, ACT_FREEFALL, 0);
}

static void tick_bounce(struct MarioState *m) {
    (void)m;
    re_timer_tick(&sBounceTimer);
}

static void register_bounce(void) {
    sBounceListenerID = REGISTER_LISTENER(PlayerLanded,    EVENT_PRIORITY_HIGH, on_player_landed);
    sBounceWallID     = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_HIGH, on_wall_bonk);
}

static void unregister_bounce(void) {
    UNREGISTER_LISTENER(PlayerLanded,    sBounceListenerID);
    UNREGISTER_LISTENER(PlayerSetAction, sBounceWallID);
}

#endif // RE_EVENTS_BOUNCE_H
