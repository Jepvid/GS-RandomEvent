#ifndef RE_EVENTS_KAIZO_H
#define RE_EVENTS_KAIZO_H

#include "game/object_helpers.h"
#include "game/interaction.h"
#include "behavior_data.h"
#include "behavior_macros.h"
#include "object_constants.h"
#include "engine/math_util.h"

// Minimal behavior — tick_kaizo handles all gameplay logic.
static const BehaviorScript bhvKaizoBlock[] = {
    BEGIN(OBJ_LIST_DEFAULT),
    // OR_INT(oFlags, ...) — oFlags raw index is 0x01; avoid OBJECT_FIELD macro in BehaviorScript context
    BC_BBH(0x11, 0x01, OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE),
    BEGIN_LOOP(),
    END_LOOP(),
};

#define KAIZO_VISIBLE_FRAMES  90
#define KAIZO_BLOCK_RADIUS    80.0f
#define KAIZO_SPAWN_HEIGHT   100.0f

static struct Object *sKaizoBlock      = NULL;
static RE_Timer       sKaizoTimer      = {0};
static RE_Timer       sKaizoSpawnDelay = {0};
static ListenerID     sKaizoJumpID;
static ListenerID     sKaizoHitID;

static void on_jump_kaizo(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    u32 act = e->action;
    if (act != ACT_JUMP && act != ACT_DOUBLE_JUMP &&
        act != ACT_TRIPLE_JUMP && act != ACT_LONG_JUMP &&
        act != ACT_BACKFLIP && act != ACT_SIDE_FLIP)
        return;
    if (is_in_castle()) return;
    if (sKaizoBlock && (sKaizoBlock->oFlags & ACTIVE_FLAG_ACTIVE)) return;
    if (re_timer_active(&sKaizoSpawnDelay)) return;
    if (rng_next() % 20 != 0) return;
    re_timer_set(&sKaizoSpawnDelay, rng_range(3, 10));
}

static void tick_kaizo(struct MarioState *m) {
    if (re_timer_active(&sKaizoSpawnDelay)) {
        if (!(m->action & ACT_FLAG_AIR)) {
            re_timer_set(&sKaizoSpawnDelay, 0); // cancel if landed
        } else if (re_timer_tick(&sKaizoSpawnDelay) == 0 && m->marioObj) {
            f32 fwdX = sins(m->faceAngle[1]) * 50.0f;
            f32 fwdZ = coss(m->faceAngle[1]) * 50.0f;
            sKaizoBlock = spawn_object_abs_with_rot(
                m->marioObj, 0,
                MODEL_EXCLAMATION_BOX, bhvKaizoBlock,
                (s16)(m->pos[0] + fwdX),
                (s16)(m->pos[1] + KAIZO_SPAWN_HEIGHT),
                (s16)(m->pos[2] + fwdZ),
                0, 0, 0
            );
            if (sKaizoBlock) {
                sKaizoBlock->hitboxRadius     = 60.0f;
                sKaizoBlock->hitboxHeight     = 60.0f;
                sKaizoBlock->hitboxDownOffset = 0.0f;
                sKaizoBlock->oInteractType    = INTERACT_BREAKABLE;
            }
            re_timer_set(&sKaizoTimer, KAIZO_VISIBLE_FRAMES);
        }
    }

    if (!sKaizoBlock) return;

    if (!(sKaizoBlock->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sKaizoBlock = NULL;
        re_timer_set(&sKaizoTimer, 0);
        return;
    }

    if (re_timer_tick(&sKaizoTimer) == 0) {
        obj_mark_for_deletion(sKaizoBlock);
        sKaizoBlock = NULL;
    }
}

// Fires the moment Mario hits any breakable — check if it's our block.
static void on_kaizo_hit(IEvent *event) {
    BreakableHit *e = (BreakableHit *)event;
    if (!sKaizoBlock || e->breakable != sKaizoBlock) return;
    if (!e->m->marioObj) return;
    spawn_object_abs_with_rot(e->m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin,
        (s16)sKaizoBlock->oPosX, (s16)sKaizoBlock->oPosY, (s16)sKaizoBlock->oPosZ, 0, 0, 0);
    obj_mark_for_deletion(sKaizoBlock);
    sKaizoBlock = NULL;
    re_timer_set(&sKaizoTimer, 0);
}

static void register_kaizo(void) {
    sKaizoJumpID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_jump_kaizo);
    sKaizoHitID  = REGISTER_LISTENER(BreakableHit,    EVENT_PRIORITY_NORMAL, on_kaizo_hit);
}

static void unregister_kaizo(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sKaizoJumpID);
    UNREGISTER_LISTENER(BreakableHit,    sKaizoHitID);
}

#endif // RE_EVENTS_KAIZO_H
