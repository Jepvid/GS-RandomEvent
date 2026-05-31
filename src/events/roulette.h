#ifndef RE_EVENTS_ROULETTE_H
#define RE_EVENTS_ROULETTE_H

#define ROULETTE_DURATION 600
#define ROULETTE_INTERVAL   5

static int sHpRoulette   = 0;
static int sCoinRoulette = 0;

static void do_hp_roulette(struct MarioState *m) {
    (void)m;
    sHpRoulette = ROULETTE_DURATION;
}

static void do_coin_roulette(struct MarioState *m) {
    (void)m;
    sCoinRoulette = ROULETTE_DURATION;
}

static void tick_roulette(struct MarioState *m) {
    if (sHpRoulette > 0) {
        if (sHpRoulette % ROULETTE_INTERVAL == 0)
            m->health = (s16)(rng_range(1, 8) * 0x100);
        sHpRoulette--;
    }

    if (sCoinRoulette > 0) {
        if (sCoinRoulette % ROULETTE_INTERVAL == 0)
            m->numCoins = (s16)rng_range(0, 999);
        sCoinRoulette--;
    }
}

#endif // RE_EVENTS_ROULETTE_H
