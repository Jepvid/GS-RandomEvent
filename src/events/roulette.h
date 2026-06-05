#ifndef RE_EVENTS_ROULETTE_H
#define RE_EVENTS_ROULETTE_H

#define ROULETTE_DURATION 600
#define ROULETTE_INTERVAL   5

static RE_Timer sHpRoulette   = {0};
static RE_Timer sCoinRoulette = {0};

static void do_hp_roulette(struct MarioState *m) {
    (void)m;
    re_timer_set(&sHpRoulette, ROULETTE_DURATION);
}

static void do_coin_roulette(struct MarioState *m) {
    (void)m;
    re_timer_set(&sCoinRoulette, ROULETTE_DURATION);
}

static void tick_roulette(struct MarioState *m) {
    if (re_timer_active(&sHpRoulette)) {
        if (sHpRoulette.remaining % ROULETTE_INTERVAL == 0)
            m->health = (s16)(rng_range(1, 8) * 0x100);
        re_timer_tick(&sHpRoulette);
    }

    if (re_timer_active(&sCoinRoulette)) {
        if (sCoinRoulette.remaining % ROULETTE_INTERVAL == 0)
            m->numCoins = (s16)rng_range(0, 999);
        re_timer_tick(&sCoinRoulette);
    }
}

#endif // RE_EVENTS_ROULETTE_H
