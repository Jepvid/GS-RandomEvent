#ifndef RE_EVENTS_MINI_MARIO_H
#define RE_EVENTS_MINI_MARIO_H

#define MINI_DURATION  600
#define MINI_SCALE     0.5f
#define MINI_JUMP_CAP  22.0f

static RE_Timer   sMiniFrames = {0};
static ListenerID sMiniHitListenerID;

static void on_mini_hit(IEvent *event) {
    if (!re_timer_active(&sMiniFrames)) return;
    PlayerHit *e = (PlayerHit *)event;
    e->m->hurtCounter += (u8)(4 * e->damage); // double damage
}

static void do_mini_mario(struct MarioState *m) {
    (void)m;
    re_timer_set(&sMiniFrames, MINI_DURATION);
}

static void tick_mini_mario(struct MarioState *m) {
    if (!re_timer_active(&sMiniFrames)) return;
    if (!m->marioObj) { re_timer_tick(&sMiniFrames); return; }

    m->marioObj->header.gfx.scale[0] = MINI_SCALE;
    m->marioObj->header.gfx.scale[1] = MINI_SCALE;
    m->marioObj->header.gfx.scale[2] = MINI_SCALE;

    if ((m->action & ACT_FLAG_AIR) && m->vel[1] > MINI_JUMP_CAP)
        m->vel[1] = MINI_JUMP_CAP;

    if (re_timer_tick(&sMiniFrames) == 0) {
        m->marioObj->header.gfx.scale[0] = 1.0f;
        m->marioObj->header.gfx.scale[1] = 1.0f;
        m->marioObj->header.gfx.scale[2] = 1.0f;
    }
}

static void register_mini_mario(void) {
    sMiniHitListenerID = REGISTER_LISTENER(PlayerHit, EVENT_PRIORITY_LOW, on_mini_hit);
}

static void unregister_mini_mario(void) {
    UNREGISTER_LISTENER(PlayerHit, sMiniHitListenerID);
}

#endif // RE_EVENTS_MINI_MARIO_H
