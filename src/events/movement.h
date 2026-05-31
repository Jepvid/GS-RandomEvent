#ifndef RE_EVENTS_MOVEMENT_H
#define RE_EVENTS_MOVEMENT_H

// Requires: rng.h, sm64.h, game/mario.h

// ---- Narcolepsy sustained state ----

static int sNarcolepsy = 0;

static void do_narcolepsy(struct MarioState *m) {
    (void)m;
    sNarcolepsy = 90; // 3 seconds at 30fps
}

// ---- Nuh-uh: block all attacks for 5 seconds ----

static int sNuhUhFrames = 0;

static void do_nuh_uh(struct MarioState *m) {
    (void)m;
    sNuhUhFrames = 150; // 5 seconds at 30fps
}

// Called every frame from on_frame_update.
static void tick_movement_states(struct MarioState *m) {
    if (sNarcolepsy > 0) {
        // Force into sleeping pose on the first frame (ground only).
        if (sNarcolepsy == 90 && !(m->action & ACT_FLAG_AIR))
            set_mario_action(m, ACT_SLEEPING, 0);
        m->input = 0;
        m->intendedMag = 0.0f;
        sNarcolepsy--;
    }

    if (sNuhUhFrames > 0) {
        // If Mario enters any attacking action, cancel it back to idle immediately.
        if (m->action & ACT_FLAG_ATTACKING)
            set_mario_action(m, ACT_IDLE, 0);
        sNuhUhFrames--;
    }
}

// ---- One-shot movement events ----

static void do_speed_boost(struct MarioState *m) {
    mario_set_forward_vel(m, 80.0f);
}

static void do_launch_up(struct MarioState *m) {
    m->vel[1] = 90.0f;
    set_mario_action(m, ACT_FREEFALL, 0);
}

static void do_squish(struct MarioState *m) {
    m->squishTimer = 120;
}

static void do_speed_reverse(struct MarioState *m) {
    mario_set_forward_vel(m, -50.0f);
}

// ACT_BURNING_FALL has ACT_FLAG_INVULNERABLE so no health is lost.
// Damage only occurs when Mario contacts lava geometry, not from the action itself.
static void do_fire(struct MarioState *m) {
    m->vel[1] = 60.0f;
    set_mario_action(m, ACT_BURNING_FALL, 0);
}

#endif // RE_EVENTS_MOVEMENT_H
