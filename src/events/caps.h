#ifndef RE_EVENTS_CAPS_H
#define RE_EVENTS_CAPS_H

#define CAP_DURATION 600 // 20 seconds

static void do_cap_wing(struct MarioState *m) {
    m->flags = (m->flags & ~MARIO_SPECIAL_CAPS) | MARIO_WING_CAP | MARIO_CAP_ON_HEAD;
    m->capTimer = CAP_DURATION;
}

static void do_cap_metal(struct MarioState *m) {
    m->flags = (m->flags & ~MARIO_SPECIAL_CAPS) | MARIO_METAL_CAP | MARIO_CAP_ON_HEAD;
    m->capTimer = CAP_DURATION;
}

static void do_cap_vanish(struct MarioState *m) {
    m->flags = (m->flags & ~MARIO_SPECIAL_CAPS) | MARIO_VANISH_CAP | MARIO_CAP_ON_HEAD;
    m->capTimer = CAP_DURATION;
}

// 15% chance: redirect any physical cap pickup to a random cap type.
static const u32 kCapFlags[3] = { MARIO_WING_CAP, MARIO_METAL_CAP, MARIO_VANISH_CAP };
static ListenerID sCapChaosID;

static void on_cap_gained(IEvent *event) {
    PlayerCapGained *e = (PlayerCapGained *)event;
    if (rng_next() % 100 >= 15) return;
    *e->capFlag = kCapFlags[rng_next() % 3];
}

static void register_caps(void) {
    sCapChaosID = REGISTER_LISTENER(PlayerCapGained, EVENT_PRIORITY_NORMAL, on_cap_gained);
}

static void unregister_caps(void) {
    UNREGISTER_LISTENER(PlayerCapGained, sCapChaosID);
}

#endif // RE_EVENTS_CAPS_H
