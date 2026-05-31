#ifndef RE_EVENTS_CANNON_H
#define RE_EVENTS_CANNON_H

// ~33% chance per cannon shot to randomise launch velocity.

static int   sPendingCannon = 0;
static f32   sCannonMult    = 1.0f;

static ListenerID sCannonListenerID;

static void on_cannon_action(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    if (e->action != ACT_SHOT_FROM_CANNON) return;
    if (rng_next() % 3 != 0) return;
    sPendingCannon = 1;
    sCannonMult    = (f32)rng_range(5, 300) / 100.0f;
}

static void tick_cannon(struct MarioState *m) {
    if (sPendingCannon && m->action == ACT_SHOT_FROM_CANNON) {
        mario_set_forward_vel(m, m->forwardVel * sCannonMult);
        m->vel[1]     *= sCannonMult;
        sPendingCannon = 0;
    }
}

static void register_cannon(void) {
    sCannonListenerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_cannon_action);
}

static void unregister_cannon(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sCannonListenerID);
}

#endif // RE_EVENTS_CANNON_H
