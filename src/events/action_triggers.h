#ifndef RE_EVENTS_ACTION_TRIGGERS_H
#define RE_EVENTS_ACTION_TRIGGERS_H

// Velocity set in tick (not listener) because PlayerSetAction fires before action init.

static int sBackflipCount     = 0;
static int sBackflipThreshold = 10;
static int sPendingBackflip   = 0;
static f32 sPendingBackflipVel = 0.0f;

static int sLongJumpCount      = 0;
static int sLongJumpThreshold  = 10;
static int sPendingLongJump    = 0;
static f32 sPendingLongJumpVel = 0.0f;

static ListenerID sActionTriggerID;

static void on_player_set_action(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;

    if (e->action == ACT_BACKFLIP) {
        sBackflipCount++;
        if (sBackflipCount >= sBackflipThreshold) {
            sBackflipCount      = 0;
            sBackflipThreshold  = rng_range(8, 15);
            sPendingBackflip    = 1;
            sPendingBackflipVel = (f32)rng_range(100, 700);
        }
    }

    if (e->action == ACT_LONG_JUMP) {
        sLongJumpCount++;
        if (sLongJumpCount >= sLongJumpThreshold) {
            sLongJumpCount      = 0;
            sLongJumpThreshold  = rng_range(8, 15);
            sPendingLongJump    = 1;
            sPendingLongJumpVel = (f32)rng_range(80, 500);
        }
    }
}

static void tick_action_triggers(struct MarioState *m) {
    if (sPendingBackflip && m->action == ACT_BACKFLIP) {
        m->vel[1]        = sPendingBackflipVel;
        sPendingBackflip = 0;
    }

    if (sPendingLongJump && m->action == ACT_LONG_JUMP) {
        mario_set_forward_vel(m, sPendingLongJumpVel);
        sPendingLongJump = 0;
    }
}

static void register_action_triggers(void) {
    sActionTriggerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_player_set_action);
}

static void unregister_action_triggers(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sActionTriggerID);
}

#endif // RE_EVENTS_ACTION_TRIGGERS_H
