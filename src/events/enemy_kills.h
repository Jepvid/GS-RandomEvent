#ifndef RE_EVENTS_ENEMY_KILLS_H
#define RE_EVENTS_ENEMY_KILLS_H

#include "game/interaction.h"
#include "game/object_helpers.h"
#include "behavior_data.h"

// INTERACT_DAMAGE covers most hostile enemies; collectibles don't carry this flag.
#define ENEMY_INTERACT_MASK (INTERACT_DAMAGE | INTERACT_BOUNCE_TOP | INTERACT_BOUNCE_TOP2)
#define EXPLOSION_RADIUS_SQ (250.0f * 250.0f)

static ListenerID sEnemyKillListenerID;

static void on_object_destroyed(IEvent *event) {
    ObjectDestroyed *e = (ObjectDestroyed *)event;
    if (!e->object || !gMarioState) return;
    if (!(e->object->oInteractType & ENEMY_INTERACT_MASK)) return;
    if (is_in_castle()) return;
    if (rng_next() % 20 != 0) return; // ~5% chance

    spawn_object_abs_with_rot(
        e->object, 0, MODEL_EXPLOSION, bhvExplosion,
        (s16)e->object->oPosX, (s16)e->object->oPosY, (s16)e->object->oPosZ,
        0, 0, 0
    );

    f32 dx = e->object->oPosX - gMarioState->pos[0];
    f32 dy = e->object->oPosY - gMarioState->pos[1];
    f32 dz = e->object->oPosZ - gMarioState->pos[2];
    if (dx*dx + dy*dy + dz*dz < EXPLOSION_RADIUS_SQ)
        hurt_and_set_mario_action(gMarioState, ACT_HARD_BACKWARD_AIR_KB, 0, 1);
}

static void register_enemy_kills(void) {
    sEnemyKillListenerID = REGISTER_LISTENER(ObjectDestroyed, EVENT_PRIORITY_NORMAL, on_object_destroyed);
}

static void unregister_enemy_kills(void) {
    UNREGISTER_LISTENER(ObjectDestroyed, sEnemyKillListenerID);
}

#endif // RE_EVENTS_ENEMY_KILLS_H
