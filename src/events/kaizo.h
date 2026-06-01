#ifndef RE_EVENTS_KAIZO_H
#define RE_EVENTS_KAIZO_H

#include "game/object_helpers.h"
#include "game/object_list_processor.h"
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

static struct Object *sKaizoBlock       = NULL;
static int            sKaizoTimer       = 0;
static int            sKaizoSpawnDelay  = 0;
static ListenerID     sKaizoListenerID;

static void on_jump_kaizo(IEvent *event) {
    PlayerSetAction *e = (PlayerSetAction *)event;
    u32 act = e->action;
    if (act != ACT_JUMP && act != ACT_DOUBLE_JUMP &&
        act != ACT_TRIPLE_JUMP && act != ACT_LONG_JUMP &&
        act != ACT_BACKFLIP && act != ACT_SIDE_FLIP)
        return;
    if (is_in_castle()) return;
    if (sKaizoBlock && (sKaizoBlock->oFlags & ACTIVE_FLAG_ACTIVE)) return;
    if (sKaizoSpawnDelay > 0) return;
    if (rng_next() % 20 != 0) return; // ~5%
    sKaizoSpawnDelay = rng_range(3, 10);
}

static void tick_kaizo(struct MarioState *m) {
    if (sKaizoSpawnDelay > 0) {
        if (!(m->action & ACT_FLAG_AIR)) {
            sKaizoSpawnDelay = 0;
        } else {
            sKaizoSpawnDelay--;
            if (sKaizoSpawnDelay == 0 && m->marioObj) {
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
                sKaizoTimer = KAIZO_VISIBLE_FRAMES;
            }
        }
    }

    if (!sKaizoBlock) return;

    if (!(sKaizoBlock->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sKaizoBlock = NULL;
        sKaizoTimer = 0;
        return;
    }

    // Engine sets INT_STATUS_INTERACTED when Mario hits the block via INTERACT_BREAKABLE.
    if (sKaizoBlock->oInteractStatus & INT_STATUS_INTERACTED) {
        sKaizoBlock->oInteractStatus = 0; // clear so it doesn't retrigger
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin,
            (s16)sKaizoBlock->oPosX, (s16)sKaizoBlock->oPosY, (s16)sKaizoBlock->oPosZ, 0, 0, 0);
    }

    sKaizoTimer--;
    if (sKaizoTimer <= 0) {
        obj_mark_for_deletion(sKaizoBlock);
        sKaizoBlock = NULL;
    }
}

static void register_kaizo(void) {
    sKaizoListenerID = REGISTER_LISTENER(PlayerSetAction, EVENT_PRIORITY_NORMAL, on_jump_kaizo);
}

static void unregister_kaizo(void) {
    UNREGISTER_LISTENER(PlayerSetAction, sKaizoListenerID);
}

#endif // RE_EVENTS_KAIZO_H
