#ifndef RE_EVENTS_CLINGY_H
#define RE_EVENTS_CLINGY_H

#include "game/object_helpers.h"

// ~17% chance on pole grab: refuse to let go for 2-10 seconds.

static ListenerID    sClingyListenerID;
static int           sClingyTimer = 0;
static struct Object *sClingyPole = NULL;

static void on_action_clingy(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    if (e->action != ACT_GRAB_POLE_SLOW && e->action != ACT_GRAB_POLE_FAST) return;
    if (rng_next() % 6 != 0) return; // ~17%
    sClingyPole  = e->m->usedObj;
    sClingyTimer = rng_range(60, 300); // 2-10 seconds
}

static void tick_clingy(struct MarioState *m) {
    if (sClingyTimer <= 0 || !sClingyPole) return;

    if (!(sClingyPole->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sClingyPole  = NULL;
        sClingyTimer = 0;
        return;
    }

    // If Mario left the pole, snap him back onto it.
    if (!(m->action & ACT_FLAG_ON_POLE)) {
        m->usedObj  = sClingyPole;
        m->pos[0]   = sClingyPole->oPosX;
        m->pos[2]   = sClingyPole->oPosZ;
        set_mario_action(m, ACT_CLIMBING_POLE, 0);
    }

    sClingyTimer--;
    if (sClingyTimer == 0)
        sClingyPole = NULL;
}

static void register_clingy(void) {
    sClingyListenerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_action_clingy);
}

static void unregister_clingy(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sClingyListenerID);
}

#endif // RE_EVENTS_CLINGY_H
