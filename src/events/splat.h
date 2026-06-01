#ifndef RE_EVENTS_SPLAT_H
#define RE_EVENTS_SPLAT_H

// ~20% chance on wall bonk: pin to wall, slide down with acceleration, land on back.

#define SPLAT_DURATION  90
#define SPLAT_PIN_FRAMES 30 // 1 second pinned before sliding

static ListenerID sSplatListenerID;
static int        sSplatFreeze = 0;
static f32        sSplatX, sSplatZ;

static void on_action_splat(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    if (e->action != ACT_SOFT_BONK) return;
    if (sSplatFreeze > 0) return;
    if (rng_next() % 5 != 0) return; // ~20%
    sSplatFreeze = SPLAT_DURATION;
    sSplatX = e->m->pos[0];
    sSplatZ = e->m->pos[2];
}

static void tick_splat(struct MarioState *m) {
    if (sSplatFreeze <= 0) return;

    m->pos[0] = sSplatX;
    m->pos[2] = sSplatZ;

    if (sSplatFreeze > (SPLAT_DURATION - SPLAT_PIN_FRAMES)) {
        // Pinned phase: fully stuck to wall.
        m->pos[1] += 0.0f;
        m->vel[1]  = 0.0f;
    } else {
        // Slide phase: quadratic acceleration downward.
        f32 slideProgress = 1.0f - (sSplatFreeze / (f32)(SPLAT_DURATION - SPLAT_PIN_FRAMES));
        m->vel[1] = -70.0f * slideProgress * slideProgress;
    }

    m->intendedMag = 0.0f;

    set_mario_animation(m, MARIO_ANIM_SHOCKED);
    set_anim_to_frame(m, 0);
    sSplatFreeze--;
}

static void register_splat(void) {
    sSplatListenerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_action_splat);
}

static void unregister_splat(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sSplatListenerID);
}

#endif // RE_EVENTS_SPLAT_H
