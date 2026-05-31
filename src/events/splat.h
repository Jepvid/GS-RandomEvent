#ifndef RE_EVENTS_SPLAT_H
#define RE_EVENTS_SPLAT_H

// ~20% chance on wall bonk (ACT_SOFT_BONK): freeze Mario in place for 3 seconds.
// Kills horizontal velocity so Mario drops straight down instead of bouncing away.

#define SPLAT_DURATION 90 // 3 seconds

static ListenerID sSplatListenerID;
static int        sSplatFreeze = 0;

static void on_action_splat(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    if (e->action != ACT_SOFT_BONK) return;
    if (rng_next() % 5 != 0) return; // 20%
    sSplatFreeze = SPLAT_DURATION;
}

static void tick_splat(struct MarioState *m) {
    if (sSplatFreeze <= 0) return;
    m->input        = 0;
    m->intendedMag  = 0.0f;
    m->vel[0]       = 0.0f;
    m->vel[2]       = 0.0f;
    m->forwardVel   = 0.0f;
    // Override whatever animation the action code set this frame and freeze at frame 0.
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
