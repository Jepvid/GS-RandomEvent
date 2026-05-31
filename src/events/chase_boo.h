#ifndef RE_EVENTS_CHASE_BOO_H
#define RE_EVENTS_CHASE_BOO_H

#include "game/object_helpers.h"
#include "behavior_data.h"

// Requires: sm64.h (MarioState, ACTIVE_FLAG_ACTIVE), game/level_update.h (WARP_OP_DEATH)

#define CHASE_BOO_DURATION  900  // 30 seconds at 30fps
#define CHASE_BOO_SPEED     6.0f
#define CHASE_KILL_RADIUS_SQ (90.0f * 90.0f)

static int sChaseBooTimer = 0;
static struct Object *sChaseBoo = NULL;

static void do_chase_boo(struct MarioState *m) {
    if (!m->marioObj) return;
    // Spawn the Boo slightly in front of Mario so the player sees it appear.
    sChaseBoo = spawn_object_abs_with_rot(
        m->marioObj, 0, MODEL_BOO, bhvBoo,
        (s16)(m->pos[0] + 400.0f), (s16)(m->pos[1] + 150.0f), (s16)(m->pos[2] + 400.0f),
        0, 0, 0
    );
    sChaseBooTimer = CHASE_BOO_DURATION;
}

static void tick_chase_boo(struct MarioState *m) {
    if (sChaseBooTimer <= 0 || !sChaseBoo) return;

    // Verify the Boo hasn't been removed by the engine.
    if (!(sChaseBoo->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sChaseBoo = NULL;
        sChaseBooTimer = 0;
        return;
    }

    // Reset health each frame so no one can kill it.
    sChaseBoo->oHealth = 0x0A;

    // Move horizontally toward Mario at constant speed.
    f32 dx = m->pos[0] - sChaseBoo->oPosX;
    f32 dz = m->pos[2] - sChaseBoo->oPosZ;
    f32 distSq = dx*dx + dz*dz;

    if (distSq > 1.0f) {
        // Normalise and scale — avoid sqrtf by dividing each axis by the larger absolute value.
        f32 adx = dx < 0.0f ? -dx : dx;
        f32 adz = dz < 0.0f ? -dz : dz;
        f32 norm = adx > adz ? adx : adz;
        f32 step = CHASE_BOO_SPEED / norm;
        sChaseBoo->oPosX += dx * step;
        sChaseBoo->oPosZ += dz * step;
    }

    // Bob up and down at Mario eye level.
    sChaseBoo->oPosY = m->pos[1] + 120.0f;

    // Kill Mario on contact.
    if (distSq < CHASE_KILL_RADIUS_SQ)
        level_trigger_warp(m, WARP_OP_DEATH);

    sChaseBooTimer--;

    // Despawn the Boo when the timer runs out.
    if (sChaseBooTimer == 0 && sChaseBoo) {
        obj_mark_for_deletion(sChaseBoo);
        sChaseBoo = NULL;
    }
}

#endif // RE_EVENTS_CHASE_BOO_H
