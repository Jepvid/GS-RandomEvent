#ifndef RE_EVENTS_SKATEBOARD_H
#define RE_EVENTS_SKATEBOARD_H

#include "game/object_helpers.h"
#include "behavior_data.h"

// Requires: sm64.h (MarioState, ACT_RIDING_SHELL_GROUND, ACT_FLAG_RIDING_SHELL),
//           game/mario.h (set_mario_action), behavior_data.h (bhvKoopaShell)

#define SKATE_DURATION 300  // 10 seconds at 30fps

static int sSkateTimer = 0;
static struct Object *sSkateShell = NULL;

static void do_skateboard(struct MarioState *m) {
    if (!m->marioObj) return;

    sSkateShell = spawn_object_abs_with_rot(
        m->marioObj, 0, MODEL_KOOPA_SHELL, bhvKoopaShell,
        (s16)m->pos[0], (s16)m->pos[1], (s16)m->pos[2],
        0, 0, 0
    );

    if (!sSkateShell) return;

    m->riddenObj = sSkateShell;
    set_mario_action(m, ACT_RIDING_SHELL_GROUND, 0);
    sSkateTimer = SKATE_DURATION;
}

static void tick_skateboard(struct MarioState *m) {
    if (sSkateTimer <= 0) return;

    // If the shell was removed or Mario dismounted, force him back on.
    if (!sSkateShell || !(sSkateShell->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sSkateShell = NULL;
        sSkateTimer = 0;
        return;
    }

    if (!(m->action & ACT_FLAG_RIDING_SHELL)) {
        m->riddenObj = sSkateShell;
        set_mario_action(m, ACT_RIDING_SHELL_GROUND, 0);
    }

    sSkateTimer--;

    if (sSkateTimer == 0) {
        // Release Mario from the shell naturally.
        m->riddenObj = NULL;
        sSkateShell = NULL;
    }
}

#endif // RE_EVENTS_SKATEBOARD_H
