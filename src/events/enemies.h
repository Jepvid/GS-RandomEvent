#ifndef RE_EVENTS_ENEMIES_H
#define RE_EVENTS_ENEMIES_H

#include "game/object_helpers.h"
#include "game/object_list_processor.h"
#include "game/interaction.h"
#include "behavior_macros.h"
#include "object_constants.h"

// Single custom behavior used for all spawned enemies.
// OBJ_LIST_PUSHABLE ensures Mario interaction (INTERACT_DAMAGE) is checked.
// The engine auto-computes oAngleToMario/oDistToMario from the OR'd flags.

static struct ObjectHitbox sChaosEnemyHitbox = {
    /* interactType:      */ INTERACT_DAMAGE,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 1,
    /* health:            */ 1,
    /* numLootCoins:      */ 0,
    /* radius:            */ 40,
    /* height:            */ 60,
    /* hurtboxRadius:     */ 30,
    /* hurtboxHeight:     */ 40,
};

static void bhv_chaos_enemy_loop(void) {
    gCurrentObject->oMoveAngleYaw = gCurrentObject->oAngleToMario;
    gCurrentObject->oForwardVel   = 8.0f;
    cur_obj_move_standard(78);
    obj_set_hitbox(gCurrentObject, &sChaosEnemyHitbox);
    cur_obj_become_tangible();
}

static const BehaviorScript bhvChaosEnemy[] = {
    BEGIN(OBJ_LIST_PUSHABLE),
    OR_INT(oFlags, OBJ_FLAG_COMPUTE_ANGLE_TO_MARIO | OBJ_FLAG_COMPUTE_DIST_TO_MARIO |
                   OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW | OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE),
    DROP_TO_FLOOR(),
    BEGIN_LOOP(),
    CALL_NATIVE(bhv_chaos_enemy_loop),
    END_LOOP(),
};

static void do_spawn_enemy(struct MarioState *m) {
    if (!m->marioObj) return;

    static const u32 kModels[] = {
        MODEL_GOOMBA,
        MODEL_KOOPA_WITH_SHELL,
        MODEL_BOBOMB_BUDDY,
    };
    static const int kModelCount = 3;

    int type  = (int)(rng_next() % (unsigned int)kModelCount);
    int count = rng_range(2, 4);

    for (int i = 0; i < count; i++) {
        s16 xOff = (s16)rng_range(-350, 350);
        s16 zOff = (s16)rng_range(-350, 350);
        spawn_object_abs_with_rot(
            m->marioObj, 0,
            kModels[type], bhvChaosEnemy,
            (s16)(m->pos[0] + xOff),
            (s16)(m->pos[1]),
            (s16)(m->pos[2] + zOff),
            0, 0, 0
        );
    }
}

#endif // RE_EVENTS_ENEMIES_H
