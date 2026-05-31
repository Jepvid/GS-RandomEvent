#ifndef RE_EVENTS_CHASE_BOO_H
#define RE_EVENTS_CHASE_BOO_H

#include "game/object_list_processor.h"
#include "game/interaction.h"

// Grabs a level enemy instead of spawning one — avoids level-specific model issues.
#define CHASE_DURATION       900
#define CHASE_SPEED          5.0f
#define CHASE_KILL_RADIUS_SQ (90.0f * 90.0f)
#define CHASE_WARN_NEAR_SQ   (250.0f * 250.0f)
#define CHASE_WARN_FAR_SQ    (500.0f * 500.0f)
#define HOSTILE_INTERACT     (INTERACT_DAMAGE | INTERACT_BOUNCE_TOP | INTERACT_BOUNCE_TOP2 | INTERACT_BULLY)

static struct Object *sChaseEnemy   = NULL;
static int            sChaseTimer   = 0;
static const char    *sChaseWarning = NULL;

static struct Object *find_level_enemy(void) {
    static const int kLists[] = { OBJ_LIST_PUSHABLE, OBJ_LIST_GENACTOR, OBJ_LIST_DESTRUCTIVE };
    for (int li = 0; li < 3; li++) {
        struct ObjectNode *list = &gObjectListArray[kLists[li]];
        struct ObjectNode *node = list->next;
        while (node != list) {
            struct Object *obj = (struct Object *)node;
            node = node->next;
            if ((obj->oFlags & ACTIVE_FLAG_ACTIVE)
                && (obj->oInteractType & HOSTILE_INTERACT)
                && obj != gMarioObject)
                return obj;
        }
    }
    return NULL;
}

static void do_chase_boo(struct MarioState *m) {
    (void)m;
    sChaseEnemy = find_level_enemy();
    if (!sChaseEnemy) return; // no enemies in level, skip silently
    sChaseTimer   = CHASE_DURATION;
    sChaseWarning = "GHOST HUNT";
}

static void tick_chase_boo(struct MarioState *m) {
    if (sChaseTimer <= 0 || !sChaseEnemy) return;

    if (!(sChaseEnemy->oFlags & ACTIVE_FLAG_ACTIVE)) {
        sChaseEnemy = sChaseWarning = NULL;
        sChaseTimer = 0;
        return;
    }

    sChaseEnemy->oHealth = 0x0A; // keep health maxed so it can't be killed

    f32 dx = m->pos[0] - sChaseEnemy->oPosX;
    f32 dz = m->pos[2] - sChaseEnemy->oPosZ;
    f32 adx = dx < 0.0f ? -dx : dx;
    f32 adz = dz < 0.0f ? -dz : dz;
    f32 norm = adx > adz ? adx : adz;
    if (norm > 0.0f) {
        // Cheap normaliser: divide by the longer axis instead of calling sqrtf.
        f32 step = CHASE_SPEED / norm;
        sChaseEnemy->oPosX += dx * step;
        sChaseEnemy->oPosZ += dz * step;
    }
    sChaseEnemy->oPosY = m->pos[1] + 50.0f;

    f32 distSq = dx*dx + dz*dz;
    sChaseWarning = distSq < CHASE_WARN_NEAR_SQ ? "RUN"
                  : distSq < CHASE_WARN_FAR_SQ  ? "GETTING CLOSER"
                  :                                "GHOST HUNT";

    if (distSq < CHASE_KILL_RADIUS_SQ)
        level_trigger_warp(m, WARP_OP_DEATH);

    sChaseTimer--;
    if (sChaseTimer == 0)
        sChaseEnemy = sChaseWarning = NULL;
}

#endif // RE_EVENTS_CHASE_BOO_H
