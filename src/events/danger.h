#ifndef RE_EVENTS_DANGER_H
#define RE_EVENTS_DANGER_H

// Requires: game/level_update.h (level_trigger_warp, WARP_OP_*)

static void do_lose_life(struct MarioState *m) {
    level_trigger_warp(m, WARP_OP_DEATH);
}

static void do_game_over(struct MarioState *m) {
    level_trigger_warp(m, WARP_OP_GAME_OVER);
}

#endif // RE_EVENTS_DANGER_H
