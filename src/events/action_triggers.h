#ifndef RE_EVENTS_ACTION_TRIGGERS_H
#define RE_EVENTS_ACTION_TRIGGERS_H

// Every 8-15 backflips → super-boost vel[1].
// Every 8-15 long jumps → randomise forwardVel via mutable event field.

static int sBackflipCount     = 0;
static int sBackflipThreshold = 10;
static int sLongJumpCount     = 0;
static int sLongJumpThreshold = 10;

static ListenerID sBackflipListenerID;
static ListenerID sLongJumpListenerID;

static void on_player_backflip(IEvent *event) {
    PlayerBackflip *e = (PlayerBackflip *)event;
    if (++sBackflipCount < sBackflipThreshold) return;
    sBackflipCount     = 0;
    sBackflipThreshold = rng_range(8, 15);
    e->m->vel[1]       = (f32)rng_range(100, 700);
}

static void on_player_long_jump(IEvent *event) {
    PlayerLongJump *e = (PlayerLongJump *)event;
    if (++sLongJumpCount < sLongJumpThreshold) return;
    sLongJumpCount     = 0;
    sLongJumpThreshold = rng_range(8, 15);
    *(e->forwardVel)   = (f32)rng_range(80, 500);
}

static void register_action_triggers(void) {
    sBackflipListenerID = REGISTER_LISTENER(PlayerBackflip, EVENT_PRIORITY_NORMAL, on_player_backflip);
    sLongJumpListenerID = REGISTER_LISTENER(PlayerLongJump, EVENT_PRIORITY_NORMAL, on_player_long_jump);
}

static void unregister_action_triggers(void) {
    UNREGISTER_LISTENER(PlayerBackflip, sBackflipListenerID);
    UNREGISTER_LISTENER(PlayerLongJump, sLongJumpListenerID);
}

#endif // RE_EVENTS_ACTION_TRIGGERS_H
