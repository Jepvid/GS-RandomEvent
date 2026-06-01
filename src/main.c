#include "mod.h"
#include "port/events/Events.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "game/print.h"
#include "sm64.h"
#include "port/api/ui.h"
#include "log/luslog.h"

#define RE_DEBUG  // uncomment to enable debug UI in settings

#include "rng.h"
#include "events/coins.h"
#include "events/movement.h"
#include "events/health.h"
#include "events/caps.h"
#include "events/danger.h"
#include "events/enemies.h"
#include "events/enemy_kills.h"
#include "events/chase_1up.h"
#include "events/skateboard.h"
#include "events/chaos.h"
#include "events/action_triggers.h"
#include "events/splat.h"
#include "events/clingy.h"
#include "events/hit_reactions.h"
#include "events/roulette.h"
#include "events/controls.h"
#include "events/wind.h"
#include "events/mini_mario.h"
#include "events/cannon.h"
#include "events/magnetism.h"
#include "events/kaizo.h"
#include "events/cam.h"
#include "events/freeze.h"

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
#define RDEV_GREEN_DEMON    18
#define RDEV_SKATEBOARD     19
#define RDEV_GRAVITY_LIGHT  20
#define RDEV_GRAVITY_HEAVY  21
#define RDEV_SLIPPERY       22
#define RDEV_NUH_UH         23
#define RDEV_HP_ROULETTE    24
#define RDEV_COIN_ROULETTE  25
#define RDEV_REV_CONTROLS   26
#define RDEV_SLOWMO         27
#define RDEV_HIGHSPEED      28
#define RDEV_WIND           29
#define RDEV_MINI_MARIO     30
#define RDEV_MAGNETISM      31
#define RDEV_CAM_TOPDOWN    32
#define RDEV_CAM_DJI        33
#define RDEV_FPS_MARIO      34
#define RDEV_FREEZE         35
#define RDEV_COUNT          36

// Normal-difficulty base weights. Damage/death events are intentionally low (1-2).
static const int kWeights[RDEV_COUNT] = {
    18, /* COIN_BONUS     */
    16, /* COIN_PENALTY   */
    10, /* SPEED_BOOST    */
     8, /* LAUNCH_UP      */
     8, /* SQUISH         */
     8, /* INVINC         */
     8, /* HEAL_TWO       */
     8, /* CAP_WING       */
     6, /* CAP_METAL      */
     6, /* CAP_VANISH     */
     6, /* NARCOLEPSY     */
     6, /* FIRE           */
     6, /* SPAWN_ENEMY    */
     4, /* SPEED_REVERSE  */
     4, /* FULL_HEAL      */
     2, /* HEALTH_DAMAGE  */
     1, /* LOSE_LIFE      */
     1, /* GAME_OVER      */
     5, /* GREEN_DEMON    */
     6, /* SKATEBOARD     */
     6, /* GRAVITY_LIGHT  */
     5, /* GRAVITY_HEAVY  */
     1, /* SLIPPERY       */
     7, /* NUH_UH         */
     4, /* HP_ROULETTE    */
     5, /* COIN_ROULETTE  */
     5, /* REV_CONTROLS   */
     6, /* SLOWMO         */
     5, /* HIGHSPEED      */
     6, /* WIND           */
     4, /* MINI_MARIO     */
     5, /* MAGNETISM      */
     5, /* CAM_TOPDOWN    */
     4, /* CAM_DJI        */
     3, /* FPS_MARIO      */
     7, /* FREEZE         */
};

// Harm level per event: 0=peaceful, 1=annoying, 2=harmful, 3=destructive
static const int kHarmLevel[RDEV_COUNT] = {
    0, /* COIN_BONUS     */
    1, /* COIN_PENALTY   */
    0, /* SPEED_BOOST    */
    0, /* LAUNCH_UP      */
    1, /* SQUISH         */
    0, /* INVINC         */
    0, /* HEAL_TWO       */
    0, /* CAP_WING       */
    0, /* CAP_METAL      */
    0, /* CAP_VANISH     */
    1, /* NARCOLEPSY     */
    2, /* FIRE           */
    2, /* SPAWN_ENEMY    */
    1, /* SPEED_REVERSE  */
    0, /* FULL_HEAL      */
    2, /* HEALTH_DAMAGE  */
    3, /* LOSE_LIFE      */
    3, /* GAME_OVER      */
    2, /* GREEN_DEMON      */
    0, /* SKATEBOARD     */
    0, /* GRAVITY_LIGHT  */
    1, /* GRAVITY_HEAVY  */
    1, /* SLIPPERY       */
    1, /* NUH_UH         */
    2, /* HP_ROULETTE    */
    1, /* COIN_ROULETTE  */
    1, /* REV_CONTROLS   */
    0, /* SLOWMO         */
    0, /* HIGHSPEED      */
    1, /* WIND           */
    2, /* MINI_MARIO     */
    1, /* MAGNETISM      */
    1, /* CAM_TOPDOWN    */
    1, /* CAM_DJI        */
    1, /* FPS_MARIO      */
    1, /* FREEZE         */
};

// Multipliers per harm level per difficulty. Pure Chaos uses weight 1 for all events.
static const int kDiffScale[5][4] = {
    { 3, 0, 0, 0 }, // 0 Peaceful
    { 2, 1, 0, 0 }, // 1 Easy
    { 1, 1, 1, 1 }, // 2 Normal
    { 1, 2, 4, 2 }, // 3 Hard
    { 1, 1, 1, 1 }, // 4 Pure Chaos (handled in get_event_weight)
};

static const char *kMessages[RDEV_COUNT] = {
    "COIN BONUS",
    "COIN PENALTY",
    "SPEED BOOST",
    "LAUNCH",
    "SQUISHED",
    "STAR POWER",
    "HEALED",
    "WING CAP",
    "METAL CAP",
    "GHOST CAP",
    "ZZZ",
    "HOT HOT HOT",
    "WATCH OUT",
    "SLIP",
    "FULL HEAL",
    "OUCH",
    "YOU DIED",
    "GAME END",
    "GREEN DEMON",
    "SKATEBOARD",
    "FLOATY",
    "CHUNKY",
    "ICY FLOORS",
    "NUH UH",
    "HP ROULETTE",
    "COIN ROULETTE",
    "BACKWARDS",
    "SLOW DOWN",
    "TURBO",
    "WIND",
    "MINI MARIO",
    "MAGNETISM",
    "TOP DOWN",
    "DJI CAM",
    "FPS MARIO",
    "FREEZE",
};

#define INTERVAL_MIN_DEFAULT   10  // seconds
#define INTERVAL_MAX_DEFAULT  120  // seconds (slider goes up to 300)
#define DISPLAY_FRAMES  120 // 4 seconds

// Two listeners so timer and action-triggered effects run independently.
static ListenerID sTimerListenerID;
static ListenerID sActionListenerID;
static ListenerID sTextListenerID;

static int sFrameCount   = 0;
static int sNextEvent    = 0;
static int sDisplayTimer = 0;
static const char *sDisplayMsg = NULL;
static int sMinIntervalNeedsSync = 0;
static int sMaxIntervalNeedsSync = 0;

static int get_event_weight(int type) {
    int diff = CVarGetInteger("gRandomEvents.Difficulty", 2);
    if (diff == 4) return 1; // Pure Chaos: all events equal
    if (diff < 0 || diff > 3) diff = 2;
    return kWeights[type] * kDiffScale[diff][kHarmLevel[type]];
}

static int pick_event(void) {
    int total = 0;
    for (int i = 0; i < RDEV_COUNT; i++)
        total += get_event_weight(i);
    if (total == 0) return 0;
    int roll = (int)(rng_next() % (unsigned int)total);
    int acc  = 0;
    for (int i = 0; i < RDEV_COUNT; i++) {
        acc += get_event_weight(i);
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
        case RDEV_GREEN_DEMON:      do_chase_1up(m);      break;
        case RDEV_SKATEBOARD:     do_skateboard(m);     break;
        case RDEV_GRAVITY_LIGHT:  do_gravity_light(m);  break;
        case RDEV_GRAVITY_HEAVY:  do_gravity_heavy(m);  break;
        case RDEV_SLIPPERY:       do_slippery(m);       break;
        case RDEV_NUH_UH:         do_nuh_uh(m);         break;
        case RDEV_HP_ROULETTE:    do_hp_roulette(m);    break;
        case RDEV_COIN_ROULETTE:  do_coin_roulette(m);  break;
        case RDEV_REV_CONTROLS:   do_rev_controls(m);   break;
        case RDEV_SLOWMO:         do_slowmo(m);         break;
        case RDEV_HIGHSPEED:      do_highspeed(m);      break;
        case RDEV_WIND:           do_wind(m);           break;
        case RDEV_MINI_MARIO:     do_mini_mario(m);     break;
        case RDEV_MAGNETISM:      do_magnetism(m);      break;
        case RDEV_CAM_TOPDOWN:    do_topdown(m);        break;
        case RDEV_CAM_DJI:        do_djicam(m);         break;
        case RDEV_FPS_MARIO:      do_fpsmario(m);       break;
        case RDEV_FREEZE:         do_freeze(m);         break;
    }
    LUSLOG_INFO("[RandomEvent] %s", kMessages[type]);
    sDisplayMsg   = kMessages[type];
    sDisplayTimer = DISPLAY_FRAMES;
}

// Advances the event timer and ticks sustained effects.
static void on_timer_update(IEvent *event) {
    if (!CVarGetInteger("gRandomEvents.Enabled", 1)) {
        sDisplayTimer = 0;
        return;
    }

    // Advance unconditionally so intangible states (bonks, deaths, cutscenes) don't stall the timer.
    sFrameCount++;
    sRandState ^= (unsigned int)sFrameCount * 2654435761u;
    if (sDisplayTimer > 0)
        sDisplayTimer--;

    if (!is_in_game()) return;

    struct MarioState *m = gMarioState;

    tick_movement_states(m);
    tick_chase_1up(m);
    tick_skateboard(m);
    tick_chaos(m);
    tick_roulette(m);
    tick_controls(m);
    tick_wind(m);
    tick_mini_mario(m);
    tick_magnetism(m);
    tick_freeze(m);

    // Enforce linked slider constraints: deferred from callbacks to avoid UI re-entrancy.
    if (sMinIntervalNeedsSync) {
        int lo = CVarGetInteger("gRandomEvents.MinInterval", INTERVAL_MIN_DEFAULT);
        int hi = CVarGetInteger("gRandomEvents.MaxInterval", INTERVAL_MAX_DEFAULT);
        if (lo > hi) CVarSetInteger("gRandomEvents.MaxInterval", lo);
        sMinIntervalNeedsSync = 0;
    }
    if (sMaxIntervalNeedsSync) {
        int lo = CVarGetInteger("gRandomEvents.MinInterval", INTERVAL_MIN_DEFAULT);
        int hi = CVarGetInteger("gRandomEvents.MaxInterval", INTERVAL_MAX_DEFAULT);
        if (hi < lo) CVarSetInteger("gRandomEvents.MinInterval", hi);
        sMaxIntervalNeedsSync = 0;
    }

    int iMin = CVarGetInteger("gRandomEvents.MinInterval", INTERVAL_MIN_DEFAULT) * 30;
    int iMax = CVarGetInteger("gRandomEvents.MaxInterval", INTERVAL_MAX_DEFAULT) * 30;

    if (sNextEvent == 0)
        sNextEvent = sFrameCount + rng_range(iMin, iMax);

    if (sFrameCount >= sNextEvent) {
        int type = pick_event();
        if (type == RDEV_GAME_OVER && m->numLives <= 0)
            type = RDEV_SQUISH;
        fire_event(type);
        sNextEvent = sFrameCount + rng_range(iMin, iMax);
    }
}

// Ticks action-triggered effects; lighter gate so they survive brief intangible frames.
static void on_action_tick(IEvent *event) {
    if (!gMarioState) return;
    struct MarioState *m = gMarioState;

    tick_action_triggers(m);
    tick_splat(m);
    tick_clingy(m);
    tick_hit_reactions(m);
    tick_cannon(m);
    tick_kaizo(m);
}

static void on_render_labels(IEvent *event) {
    if (sDisplayTimer > 0 && sDisplayMsg)
        print_text_centered(160, 140, sDisplayMsg);
    if (sChaseWarning)
        print_text_centered(160, 165, sChaseWarning);
}

static const C_ComboboxOption kDiffOptions[] = {
    { 0, "Peaceful"   },
    { 1, "Easy"       },
    { 2, "Normal"     },
    { 3, "Hard"       },
    { 4, "Pure Chaos" },
    { 0, NULL         },
};

static void on_min_interval_changed(void) {
    sMinIntervalNeedsSync = 1;
}

static void on_max_interval_changed(void) {
    sMaxIntervalNeedsSync = 1;
}

#ifdef RE_DEBUG
static void re_debug_fire_event(int idx) {
    if (!is_in_game()) return;
    int type = idx;
    if (type == RDEV_GAME_OVER && gMarioState->numLives <= 0)
        type = RDEV_SQUISH;
    fire_event(type);
    int iMin = CVarGetInteger("gRandomEvents.MinInterval", INTERVAL_MIN_DEFAULT) * 30;
    int iMax = CVarGetInteger("gRandomEvents.MaxInterval", INTERVAL_MAX_DEFAULT) * 30;
    sNextEvent = sFrameCount + rng_range(iMin, iMax);
}

#define DEF_EVENT_FIRE(n) static void re_fire_##n(void) { re_debug_fire_event(n); }
DEF_EVENT_FIRE( 0) DEF_EVENT_FIRE( 1) DEF_EVENT_FIRE( 2) DEF_EVENT_FIRE( 3) DEF_EVENT_FIRE( 4)
DEF_EVENT_FIRE( 5) DEF_EVENT_FIRE( 6) DEF_EVENT_FIRE( 7) DEF_EVENT_FIRE( 8) DEF_EVENT_FIRE( 9)
DEF_EVENT_FIRE(10) DEF_EVENT_FIRE(11) DEF_EVENT_FIRE(12) DEF_EVENT_FIRE(13) DEF_EVENT_FIRE(14)
DEF_EVENT_FIRE(15) DEF_EVENT_FIRE(16) DEF_EVENT_FIRE(17) DEF_EVENT_FIRE(18) DEF_EVENT_FIRE(19)
DEF_EVENT_FIRE(20) DEF_EVENT_FIRE(21) DEF_EVENT_FIRE(22) DEF_EVENT_FIRE(23) DEF_EVENT_FIRE(24)
DEF_EVENT_FIRE(25) DEF_EVENT_FIRE(26) DEF_EVENT_FIRE(27) DEF_EVENT_FIRE(28) DEF_EVENT_FIRE(29)
DEF_EVENT_FIRE(30) DEF_EVENT_FIRE(31) DEF_EVENT_FIRE(32) DEF_EVENT_FIRE(33) DEF_EVENT_FIRE(34)
#undef DEF_EVENT_FIRE

static void (*const kDebugFire[RDEV_COUNT])(void) = {
    re_fire_0,  re_fire_1,  re_fire_2,  re_fire_3,  re_fire_4,
    re_fire_5,  re_fire_6,  re_fire_7,  re_fire_8,  re_fire_9,
    re_fire_10, re_fire_11, re_fire_12, re_fire_13, re_fire_14,
    re_fire_15, re_fire_16, re_fire_17, re_fire_18, re_fire_19,
    re_fire_20, re_fire_21, re_fire_22, re_fire_23, re_fire_24,
    re_fire_25, re_fire_26, re_fire_27, re_fire_28, re_fire_29,
    re_fire_30, re_fire_31, re_fire_32, re_fire_33, re_fire_34,
};
#endif

static void setup_ui(void) {
    C_AddSidebarEntry("Random Events", 1);

    C_WidgetConfig en = {0};
    en.type = C_WIDGET_CVAR_CHECKBOX;
    en.cvar = "gRandomEvents.Enabled";
    en.opts.checkbox.tooltip = "Enable random events during gameplay.";
    en.opts.checkbox.default_val = true;
    C_AddWidget("Random Events", 1, "Enable", &en);

    C_WidgetConfig diff = {0};
    diff.type = C_WIDGET_CVAR_COMBOBOX;
    diff.cvar = "gRandomEvents.Difficulty";
    diff.opts.combo.tooltip = "Peaceful: no harmful events. Easy: no damage. Normal: default. Hard: more danger. Pure Chaos: all events equally likely.";
    diff.opts.combo.default_index = 2;
    diff.opts.combo.map = (C_ComboboxOption*)kDiffOptions;
    C_AddWidget("Random Events", 1, "Difficulty", &diff);

    C_WidgetConfig sep1 = {0};
    sep1.type = C_WIDGET_SEPARATOR_TEXT;
    C_AddWidget("Random Events", 1, "Timer", &sep1);

    C_WidgetConfig minS = {0};
    minS.type = C_WIDGET_CVAR_SLIDER_INT;
    minS.cvar = "gRandomEvents.MinInterval";
    minS.opts.slider_int.min = 10;
    minS.opts.slider_int.max = 300;
    minS.opts.slider_int.step = 5;
    minS.opts.slider_int.default_val = INTERVAL_MIN_DEFAULT;
    minS.opts.slider_int.format = "%ds";
    minS.opts.slider_int.tooltip = "Minimum time between random events.";
    minS.callback = on_min_interval_changed;
    C_AddWidget("Random Events", 1, "Min Interval", &minS);

    C_WidgetConfig maxS = {0};
    maxS.type = C_WIDGET_CVAR_SLIDER_INT;
    maxS.cvar = "gRandomEvents.MaxInterval";
    maxS.opts.slider_int.min = 10;
    maxS.opts.slider_int.max = 300;
    maxS.opts.slider_int.step = 5;
    maxS.opts.slider_int.default_val = INTERVAL_MAX_DEFAULT;
    maxS.opts.slider_int.format = "%ds";
    maxS.opts.slider_int.tooltip = "Maximum time between random events.";
    maxS.callback = on_max_interval_changed;
    C_AddWidget("Random Events", 1, "Max Interval", &maxS);

#ifdef RE_DEBUG
    C_WidgetConfig sep2 = {0};
    sep2.type = C_WIDGET_SEPARATOR_TEXT;
    C_AddWidget("Random Events", 1, "Debug", &sep2);

    for (int i = 0; i < RDEV_COUNT; i++) {
        C_WidgetConfig btn = {0};
        btn.type     = C_WIDGET_BUTTON;
        btn.callback = kDebugFire[i];
        btn.opts.generic.tooltip = "Fire this event immediately (must be in-game).";
        C_AddWidget("Random Events", 1, kMessages[i], &btn);
    }
#endif
}

MOD_INIT() {
    setup_ui();
    sTimerListenerID  = REGISTER_LISTENER(GameFrameUpdate,  EVENT_PRIORITY_NORMAL, on_timer_update);
    sActionListenerID = REGISTER_LISTENER(GameFrameUpdate,  EVENT_PRIORITY_LOW,    on_action_tick);
    sTextListenerID   = REGISTER_LISTENER(RenderTextLabels, EVENT_PRIORITY_NORMAL, on_render_labels);
    register_action_triggers();
    register_enemy_kills();
    register_chase_1up();
    register_splat();
    register_clingy();
    register_hit_reactions();
    register_mini_mario();
    register_cannon();
    register_kaizo();
    register_cam();
}

MOD_EXIT() {
    C_RemoveSidebarEntry("Random Events");
    UNREGISTER_LISTENER(GameFrameUpdate,  sTimerListenerID);
    UNREGISTER_LISTENER(GameFrameUpdate,  sActionListenerID);
    UNREGISTER_LISTENER(RenderTextLabels, sTextListenerID);
    unregister_action_triggers();
    unregister_enemy_kills();
    unregister_chase_1up();
    unregister_splat();
    unregister_clingy();
    unregister_hit_reactions();
    unregister_mini_mario();
    unregister_cannon();
    unregister_kaizo();
    unregister_cam();
}
