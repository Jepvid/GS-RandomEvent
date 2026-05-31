#include "mod.h"
#include "port/events/Events.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "game/print.h"
#include "sm64.h"
#include "port/api/ui.h"

#include "rng.h"
#include "events/coins.h"
#include "events/movement.h"       // do_narcolepsy, do_fire, do_squish, ..., tick_movement_states
#include "events/health.h"
#include "events/caps.h"
#include "events/danger.h"
#include "events/enemies.h"        // do_spawn_enemy
#include "events/enemy_kills.h"    // ObjectDestroyed listener (5% explosion)
#include "events/chase_boo.h"      // do_chase_boo, tick_chase_boo
#include "events/skateboard.h"     // do_skateboard, tick_skateboard
#include "events/chaos.h"          // do_gravity_light/heavy, do_slippery, tick_chaos
#include "events/action_triggers.h" // PlayerSetAction listener

// ---- Event type IDs ----

#define RDEV_COIN_BONUS      0
#define RDEV_COIN_PENALTY    1
#define RDEV_SPEED_BOOST     2
#define RDEV_LAUNCH_UP       3
#define RDEV_SQUISH          4
#define RDEV_INVINC          5
#define RDEV_HEAL_TWO        6
#define RDEV_CAP_WING        7
#define RDEV_CAP_METAL       8
#define RDEV_CAP_VANISH      9
#define RDEV_NARCOLEPSY     10
#define RDEV_FIRE           11
#define RDEV_SPAWN_ENEMY    12
#define RDEV_SPEED_REVERSE  13
#define RDEV_FULL_HEAL      14
#define RDEV_HEALTH_DAMAGE  15
#define RDEV_LOSE_LIFE      16
#define RDEV_GAME_OVER      17
#define RDEV_CHASE_BOO      18
#define RDEV_SKATEBOARD     19
#define RDEV_GRAVITY_LIGHT  20
#define RDEV_GRAVITY_HEAVY  21
#define RDEV_SLIPPERY       22
#define RDEV_NUH_UH         23
#define RDEV_COUNT          24

// Higher = more frequent. Total = 156.
// Dangerous events (damage / death) are kept at weight 1-2.
// Slippery is weight 1 — too disruptive at higher frequency.
static const int kWeights[RDEV_COUNT] = {
    18, /* COIN_BONUS      common    */
    16, /* COIN_PENALTY    common    */
    10, /* SPEED_BOOST     common    */
     8, /* LAUNCH_UP       common    */
     8, /* SQUISH          common    */
     8, /* INVINC          common    */
     8, /* HEAL_TWO        common    */
     8, /* CAP_WING        common    */
     6, /* CAP_METAL       uncommon  */
     6, /* CAP_VANISH      uncommon  */
     6, /* NARCOLEPSY      uncommon  */
     6, /* FIRE            uncommon  */
     6, /* SPAWN_ENEMY     uncommon  */
     4, /* SPEED_REVERSE   uncommon  */
     4, /* FULL_HEAL       uncommon  */
     2, /* HEALTH_DAMAGE   rare      */
     1, /* LOSE_LIFE       rare      */
     1, /* GAME_OVER       very rare */
     5, /* CHASE_BOO       uncommon  */
     6, /* SKATEBOARD      uncommon  */
     6, /* GRAVITY_LIGHT   uncommon  */
     5, /* GRAVITY_HEAVY   uncommon  */
     1, /* SLIPPERY        very rare */
     7, /* NUH_UH          uncommon  */
};

#define WEIGHT_TOTAL 156

// SM64 HUD font: A-Z, 0-9, space, limited punctuation.
static const char *kMessages[RDEV_COUNT] = {
    "COIN BONUS",
    "COIN PENALTY",
    "SPEED BOOST",
    "LAUNCH",
    "SQUISHED",
    "INVINCIBLE",
    "HEALED",
    "WING CAP",
    "METAL CAP",
    "VANISH CAP",
    "ZZZ",
    "HOT HOT HOT",
    "WATCH OUT",
    "SLIP",
    "FULL HEAL",
    "OUCH",
    "YOU DIED",
    "GAME OVER",
    "GHOST HUNT",
    "SKATEBOARD",
    "LOW GRAVITY",
    "HEAVY",
    "ICY FLOORS",
    "NUH UH",
};

// Events fire every 40-100 seconds (1200-3000 frames at 30fps).
#define INTERVAL_MIN 1200
#define INTERVAL_MAX 3000
// Message stays on screen for 4 seconds (120 frames).
#define DISPLAY_FRAMES 120

// ---- Listener handles ----

static ListenerID sFrameListenerID;
static ListenerID sTextListenerID;

// ---- Runtime state ----

static int sFrameCount   = 0;
static int sNextEvent    = 0;
static int sDisplayTimer = 0;
static const char *sDisplayMsg = NULL;

// ---- Helpers ----

static int pick_event(void) {
    int roll = (int)(rng_next() % (unsigned int)WEIGHT_TOTAL);
    int acc = 0;
    for (int i = 0; i < RDEV_COUNT; i++) {
        acc += kWeights[i];
        if (roll < acc) return i;
    }
    return 0;
}

static int is_in_game(void) {
    return gMarioState
        && gMarioState->action != ACT_UNINITIALIZED
        && !(gMarioState->action & ACT_FLAG_INTANGIBLE);
}

static void fire_event(int type) {
    struct MarioState *m = gMarioState;
    switch (type) {
        case RDEV_COIN_BONUS:     do_coin_bonus(m);     break;
        case RDEV_COIN_PENALTY:   do_coin_penalty(m);   break;
        case RDEV_SPEED_BOOST:    do_speed_boost(m);    break;
        case RDEV_LAUNCH_UP:      do_launch_up(m);      break;
        case RDEV_SQUISH:         do_squish(m);         break;
        case RDEV_INVINC:         do_invinc(m);         break;
        case RDEV_HEAL_TWO:       do_heal_two(m);       break;
        case RDEV_CAP_WING:       do_cap_wing(m);       break;
        case RDEV_CAP_METAL:      do_cap_metal(m);      break;
        case RDEV_CAP_VANISH:     do_cap_vanish(m);     break;
        case RDEV_NARCOLEPSY:     do_narcolepsy(m);     break;
        case RDEV_FIRE:           do_fire(m);           break;
        case RDEV_SPAWN_ENEMY:    do_spawn_enemy(m);    break;
        case RDEV_SPEED_REVERSE:  do_speed_reverse(m);  break;
        case RDEV_FULL_HEAL:      do_full_heal(m);      break;
        case RDEV_HEALTH_DAMAGE:  do_health_damage(m);  break;
        case RDEV_LOSE_LIFE:      do_lose_life(m);      break;
        case RDEV_GAME_OVER:      do_game_over(m);      break;
        case RDEV_CHASE_BOO:      do_chase_boo(m);      break;
        case RDEV_SKATEBOARD:     do_skateboard(m);     break;
        case RDEV_GRAVITY_LIGHT:  do_gravity_light(m);  break;
        case RDEV_GRAVITY_HEAVY:  do_gravity_heavy(m);  break;
        case RDEV_SLIPPERY:       do_slippery(m);       break;
        case RDEV_NUH_UH:         do_nuh_uh(m);         break;
    }
    sDisplayMsg   = kMessages[type];
    sDisplayTimer = DISPLAY_FRAMES;
}

// ---- Listeners ----

static void on_frame_update(IEvent *event) {
    if (!CVarGetInteger("gRandomEvents.Enabled", 1)) {
        sDisplayTimer = 0;
        return;
    }
    if (!is_in_game()) return;

    struct MarioState *m = gMarioState;

    // Sustain ongoing effects every frame.
    tick_movement_states(m);
    tick_chase_boo(m);
    tick_skateboard(m);
    tick_chaos(m);

    sFrameCount++;
    // Stir RNG so trigger timing affects randomness.
    sRandState ^= (unsigned int)sFrameCount * 2654435761u;

    if (sNextEvent == 0)
        sNextEvent = sFrameCount + rng_range(INTERVAL_MIN, INTERVAL_MAX);

    if (sFrameCount >= sNextEvent) {
        int type = pick_event();
        // Never game-over if on the last life.
        if (type == RDEV_GAME_OVER && m->numLives <= 0)
            type = RDEV_SQUISH;
        fire_event(type);
        sNextEvent = sFrameCount + rng_range(INTERVAL_MIN, INTERVAL_MAX);
    }

    if (sDisplayTimer > 0)
        sDisplayTimer--;
}

static void on_render_labels(IEvent *event) {
    if (sDisplayTimer > 0 && sDisplayMsg)
        print_text_centered(160, 140, sDisplayMsg);
}

// ---- UI ----

static void setup_ui(void) {
    C_AddSidebarEntry("Random Events", 1);

    C_WidgetConfig en = {0};
    en.type = C_WIDGET_CVAR_CHECKBOX;
    en.cvar = "gRandomEvents.Enabled";
    en.opts.checkbox.tooltip = "Enable random events during gameplay.";
    en.opts.checkbox.default_val = true;
    C_AddWidget("Random Events", 1, "Enable", &en);
}

// ---- Entry points ----

MOD_INIT() {
    setup_ui();
    sFrameListenerID = REGISTER_LISTENER(GameFrameUpdate,  EVENT_PRIORITY_NORMAL, on_frame_update);
    sTextListenerID  = REGISTER_LISTENER(RenderTextLabels, EVENT_PRIORITY_NORMAL, on_render_labels);
    register_action_triggers();
    register_enemy_kills();
}

MOD_EXIT() {
    C_RemoveSidebarEntry("Random Events");
    UNREGISTER_LISTENER(GameFrameUpdate,  sFrameListenerID);
    UNREGISTER_LISTENER(RenderTextLabels, sTextListenerID);
    unregister_action_triggers();
    unregister_enemy_kills();
}
