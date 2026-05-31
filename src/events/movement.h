#ifndef RE_EVENTS_MOVEMENT_H
#define RE_EVENTS_MOVEMENT_H

static int sNarcolepsy  = 0;
static int sNuhUhFrames = 0;

static void do_narcolepsy(struct MarioState *m) {
    (void)m;
    sNarcolepsy = 90; // 3 seconds
}

static void do_nuh_uh(struct MarioState *m) {
    (void)m;
    sNuhUhFrames = 150; // 5 seconds
}

static void tick_movement_states(struct MarioState *m) {
    if (sNarcolepsy > 0) {
        // Force sleep pose only on the first frame; == 90 is the initial value.
        if (sNarcolepsy == 90 && !(m->action & ACT_FLAG_AIR))
            set_mario_action(m, ACT_SLEEPING, 0);
        m->input = 0;
        m->intendedMag = 0.0f;
        sNarcolepsy--;
    }

    if (sNuhUhFrames > 0) {
        if (m->action & ACT_FLAG_ATTACKING)
            set_mario_action(m, ACT_IDLE, 0);
        sNuhUhFrames--;
    }
}

static void do_speed_boost(struct MarioState *m) {
    mario_set_forward_vel(m, 80.0f);
}

static void do_launch_up(struct MarioState *m) {
    set_mario_action(m, ACT_FREEFALL, 0);
    m->vel[1] = 90.0f;
}

static void do_squish(struct MarioState *m) {
    m->squishTimer = 120;
}

static void do_speed_reverse(struct MarioState *m) {
    mario_set_forward_vel(m, -50.0f);
}

// ACT_BURNING_FALL carries ACT_FLAG_INVULNERABLE — lava damage only triggers from
// geometry contact, not from the action itself, so no health is lost.
static void do_fire(struct MarioState *m) {
    set_mario_action(m, ACT_BURNING_FALL, 0);
    m->vel[1] = 60.0f;
}

#endif // RE_EVENTS_MOVEMENT_H
