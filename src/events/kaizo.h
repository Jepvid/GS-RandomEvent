#ifndef RE_EVENTS_KAIZO_H
#define RE_EVENTS_KAIZO_H

#include "game/object_helpers.h"
#include "behavior_data.h"
#include "behavior_macros.h"
#include "object_constants.h"
#include "engine/math_util.h"

// Minimal behavior: exists in OBJ_LIST_DEFAULT, updates its gfx position, does nothing else.
// All gameplay logic (proximity bonk, coin, despawn) is handled by tick_kaizo.
static const BehaviorScript bhvKaizoBlock[] = {
    BEGIN(OBJ_LIST_DEFAULT),
    OR_INT(oFlags, OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE),
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

    if ((m->action & ACT_FLAG_AIR) && m->vel[1] > 0.0f) {
        f32 dx  = m->pos[0] - sKaizoBlock->oPosX;
        f32 dz  = m->pos[2] - sKaizoBlock->oPosZ;
        f32 dy  = m->pos[1] - sKaizoBlock->oPosY;
        f32 adx = dx < 0.0f ? -dx : dx;
        f32 adz = dz < 0.0f ? -dz : dz;
        if (adx < KAIZO_BLOCK_RADIUS && adz < KAIZO_BLOCK_RADIUS &&
            dy > -40.0f && dy < 80.0f) {
            m->vel[1] = -12.0f;
            set_mario_action(m, ACT_SOFT_BONK, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin,
                (s16)sKaizoBlock->oPosX, (s16)sKaizoBlock->oPosY, (s16)sKaizoBlock->oPosZ, 0, 0, 0);
        }
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
