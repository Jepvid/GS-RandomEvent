#ifndef RE_EVENTS_ACTION_TRIGGERS_H
#define RE_EVENTS_ACTION_TRIGGERS_H

// Requires: rng.h, sm64.h, game/mario.h, port/events/Events.h

// ---- Backflip: every 8-15 backflips Mario gets launched into the sky ----

static int sBackflipCount     = 0;
static int sBackflipThreshold = 10;

// ---- Long jump: every 8-15 long jumps Mario rockets horizontally ----

static int sLongJumpCount     = 0;
static int sLongJumpThreshold = 10;

static ListenerID sActionTriggerID;

static void on_player_set_action(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    struct MarioState *m = e->m;

    if (e->action == ACT_BACKFLIP) {
        sBackflipCount++;
        if (sBackflipCount >= sBackflipThreshold) {
            sBackflipCount     = 0;
            sBackflipThreshold = rng_range(8, 15);
            m->vel[1] = 130.0f;
        }
    }

    if (e->action == ACT_LONG_JUMP) {
        sLongJumpCount++;
        if (sLongJumpCount >= sLongJumpThreshold) {
            sLongJumpCount     = 0;
            sLongJumpThreshold = rng_range(8, 15);
            mario_set_forward_vel(m, 200.0f);
        }
    }
}

static void register_action_triggers(void) {
    sActionTriggerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_player_set_action);
}

static void unregister_action_triggers(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sActionTriggerID);
}

#endif // RE_EVENTS_ACTION_TRIGGERS_H
