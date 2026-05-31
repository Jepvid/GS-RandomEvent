#ifndef RE_EVENTS_ENEMIES_H
#define RE_EVENTS_ENEMIES_H

#include "game/object_helpers.h"
#include "behavior_data.h"

static void do_spawn_enemy(struct MarioState *m) {
    if (!m->marioObj) return;

    static const struct { u32 model; const BehaviorScript *bhv; } kEnemies[] = {
        { MODEL_GOOMBA,           bhvGoomba },
        { MODEL_KOOPA_WITH_SHELL, bhvKoopa  },
        { MODEL_BOBOMB_BUDDY,     bhvBobomb },
    };
    static const int kEnemyCount = 3;

    int type  = (int)(rng_next() % (unsigned int)kEnemyCount);
    int count = rng_range(2, 4);

    for (int i = 0; i < count; i++) {
        s16 xOff = (s16)rng_range(-350, 350);
        s16 zOff = (s16)rng_range(-350, 350);
        spawn_object_abs_with_rot(
            m->marioObj, 0,
            kEnemies[type].model, kEnemies[type].bhv,
            (s16)(m->pos[0] + xOff),
            (s16)(m->pos[1]),
            (s16)(m->pos[2] + zOff),
            0, 0, 0
        );
    }
}

#endif // RE_EVENTS_ENEMIES_H
