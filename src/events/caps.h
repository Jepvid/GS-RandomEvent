#ifndef RE_EVENTS_CAPS_H
#define RE_EVENTS_CAPS_H

// Requires: sm64.h (MARIO_WING_CAP, MARIO_METAL_CAP, MARIO_VANISH_CAP,
//           MARIO_SPECIAL_CAPS, MARIO_CAP_ON_HEAD, MarioState)

#define CAP_DURATION 600 // 20 seconds at 30fps

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

#endif // RE_EVENTS_CAPS_H
