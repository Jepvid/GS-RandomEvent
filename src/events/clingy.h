#ifndef RE_EVENTS_CLINGY_H
#define RE_EVENTS_CLINGY_H

#include "game/object_helpers.h"

// ~17% chance on pole grab: refuse to let go for 2-10 seconds.

static ListenerID    sClingyListenerID;
static RE_Timer      sClingyTimer = {0};
static struct Object *sClingyPole = NULL;

static void on_pole_grabbed(IEvent *event) {
    PoleGrabbed *e = (PoleGrabbed *)event;
    if (rng_next() % 6 != 0) return;
    sClingyPole = e->pole;
    re_timer_set(&sClingyTimer, rng_range(60, 300));
}

static void tick_clingy(struct MarioState *m) {
    if (!re_timer_active(&sClingyTimer) || !sClingyPole) return;

    if (!(sClingyPole->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sClingyPole  = NULL;
        re_timer_set(&sClingyTimer, 0);
        return;
    }

    if (!(m->action & ACT_FLAG_ON_POLE)) {
        m->usedObj  = sClingyPole;
        m->pos[0]   = sClingyPole->oPosX;
        m->pos[2]   = sClingyPole->oPosZ;
        set_mario_action(m, ACT_CLIMBING_POLE, 0);
    }

    if (re_timer_tick(&sClingyTimer) == 0)
        sClingyPole = NULL;
}

static void register_clingy(void) {
    sClingyListenerID = REGISTER_LISTENER(PoleGrabbed, EVENT_PRIORITY_NORMAL, on_pole_grabbed);
}

static void unregister_clingy(void) {
    UNREGISTER_LISTENER(PoleGrabbed, sClingyListenerID);
}

#endif // RE_EVENTS_CLINGY_H
