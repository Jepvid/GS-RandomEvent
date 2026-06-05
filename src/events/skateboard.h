#ifndef RE_EVENTS_SKATEBOARD_H
#define RE_EVENTS_SKATEBOARD_H

#include "game/object_helpers.h"
#include "behavior_data.h"

#define SKATE_DURATION 300 // 10 seconds

static RE_Timer       sSkateTimer = {0};
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
    re_timer_set(&sSkateTimer, SKATE_DURATION);
}

static void tick_skateboard(struct MarioState *m) {
    if (!re_timer_active(&sSkateTimer)) return;

    if (!sSkateShell || !(sSkateShell->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sSkateShell = NULL;
        re_timer_set(&sSkateTimer, 0);
        return;
    }

    if (!(m->action & ACT_FLAG_RIDING_SHELL)) {
        m->riddenObj = sSkateShell;
        set_mario_action(m, ACT_RIDING_SHELL_GROUND, 0);
    }

    if (re_timer_tick(&sSkateTimer) == 0) {
        m->riddenObj = NULL;
        sSkateShell  = NULL;
    }
}

#endif // RE_EVENTS_SKATEBOARD_H
