#ifndef RE_EVENTS_CHASE_1UP_H
#define RE_EVENTS_CHASE_1UP_H

#include "game/object_helpers.h"
#include "behavior_data.h"

// Uses bhvHidden1upInPole which contains pole_1up_move_towards_mario() in action 1.
// Spawning with action=1 skips the hidden/trigger phase and starts the chase immediately.
// The mushroom flies toward Mario using the engine's own pitch/yaw approach logic.
//
// Collection fires CALL_CANCELLABLE_EVENT(PlayerLivesChange) — we intercept that,
// cancel the life gain, and kill Mario instead. No manual position override needed.

#define CHASE_DURATION     900
#define CHASE_WARN_NEAR_SQ (250.0f * 250.0f)
#define CHASE_WARN_FAR_SQ  (500.0f * 500.0f)

static struct Object *sChase1Up     = NULL;
static int            sChaseTimer   = 0;
static const char    *sChaseWarning = NULL;

static ListenerID sGreenDemonLivesListenerID;

static void on_lives_change_green_demon(IEvent *event) {
    PlayerLivesChange *e = (PlayerLivesChange *)event;
    if (!sChase1Up || sChaseTimer <= 0) return;
    if (e->lives <= 0) return; // only intercept life gains (+1 from 1up collect)

    // The demon caught Mario — cancel the life and kill him instead.
    e->Event.Cancelled = true;
    obj_mark_for_deletion(sChase1Up);
    sChase1Up     = NULL;
    sChaseTimer   = 0;
    sChaseWarning = NULL;
    level_trigger_warp(e->m, WARP_OP_DEATH);
}

static void do_chase_1up(struct MarioState *m) {
    if (!m->marioObj) return;
    sChase1Up = spawn_object_abs_with_rot(
        m->marioObj, 0, MODEL_1UP, bhvHidden1upInPole,
        (s16)(m->pos[0] + 400.0f),
        (s16)(m->pos[1] + 200.0f),
        (s16)(m->pos[2] + 400.0f),
        0, 0, 0
    );
    if (!sChase1Up) return;
    sChase1Up->oAction = 1; // skip hidden/trigger, begin chasing immediately
    sChaseTimer   = CHASE_DURATION;
    sChaseWarning = "GREEN DEMON";
}

static void tick_chase_1up(struct MarioState *m) {
    if (sChaseTimer <= 0 || !sChase1Up) return;

    if (!(sChase1Up->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sChase1Up = sChaseWarning = NULL;
        sChaseTimer = 0;
        return;
    }

    f32 dx = m->pos[0] - sChase1Up->oPosX;
    f32 dz = m->pos[2] - sChase1Up->oPosZ;
    f32 distSq = dx*dx + dz*dz;
    sChaseWarning = distSq < CHASE_WARN_NEAR_SQ ? "RUN"
                  : distSq < CHASE_WARN_FAR_SQ  ? "IT FOLLOWS"
                  :                                "GREEN DEMON";

    sChaseTimer--;
    if (sChaseTimer == 0) {
        obj_mark_for_deletion(sChase1Up);
        sChase1Up = sChaseWarning = NULL;
    }
}

static void register_chase_1up(void) {
    sGreenDemonLivesListenerID = REGISTER_LISTENER(PlayerLivesChange, EVENT_PRIORITY_HIGH, on_lives_change_green_demon);
}

static void unregister_chase_1up(void) {
    UNREGISTER_LISTENER(PlayerLivesChange, sGreenDemonLivesListenerID);
}

#endif // RE_EVENTS_CHASE_1UP_H
