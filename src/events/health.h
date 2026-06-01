#ifndef RE_EVENTS_HEALTH_H
#define RE_EVENTS_HEALTH_H

// Health: 0x880 = full (8 pips), 0x100 = 1 pip.

static void do_heal_two(struct MarioState *m) {
    m->health += 0x200;
    if (m->health > 0x880) m->health = 0x880;
}

static void do_health_damage(struct MarioState *m) {
    if (m->health > 0x200) {
        m->health -= 0x100;
        m->hurtCounter = 4;
    }
}

static void do_full_heal(struct MarioState *m) {
    m->health = 0x880;
}

static void do_invinc(struct MarioState *m) {
    m->invincTimer = 180;
}

#endif // RE_EVENTS_HEALTH_H
