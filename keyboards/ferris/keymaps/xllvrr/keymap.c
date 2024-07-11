#include QMK_KEYBOARD_H

// initialise variables required for macros and tap dances
bool is_alt_tab_active = false;
uint16_t alt_tab_timer = 0;

// initialise layer names
enum my_layers {
    BASE_LAYER,
    SYM_LAYER,
    NUM_LAYER,
    FNV_LAYER,
    SYS_LAYER
};

// initialise tap dances
enum my_tapdances {
    CTL_BSPC
};

typedef enum {
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_DOUBLE_HOLD
} td_state_t;

typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;

td_state_t cur_dance(tap_dance_state_t *state);

// for the ctrl + backspace
void cbs_finished(tap_dance_state_t *state, void *user_data);
void cbs_reset(tap_dance_state_t *state, void *user_data);

// initialise combos
/*
Understanding that the layout is mainly based off Colemak DHm:
Q+W = Esc
Q+F = Tab
Chording the Layer Switch buttons invokes either the function/nav layer from base or returns to base from any other layer
*/
enum combos {
    QW_ESC,
    QF_TAB,
    REBASE,
    SYSWIN
};

const uint16_t PROGMEM qw_combo[] = {KC_Q, KC_W, COMBO_END};
const uint16_t PROGMEM qf_combo[] = {KC_Q, KC_F, COMBO_END};
const uint16_t PROGMEM retobase[] = {OSM(MOD_LSFT), LT(NUM_LAYER,KC_ENT), COMBO_END};
const uint16_t PROGMEM syswinop[] = {OSM(MOD_LSFT), KC_SPC, KC_BSPC, LT(NUM_LAYER,KC_ENT), COMBO_END};

combo_t key_combos[] = {
    [QW_ESC] = COMBO(qw_combo, KC_ESC),
    [QF_TAB] = COMBO(qf_combo, KC_TAB),
    [REBASE] = COMBO(retobase, TO(BASE_LAYER)),
    [SYSWIN] = COMBO(syswinop, TO(SYS_LAYER))
};

// initialise macros
/*
Super Alt-Tab
Windows Specific:
    Hibernate (GUI + X, U + H)
*/
enum my_keycodes{
    ALT_TAB = SAFE_RANGE,
    HIBERNATE
};

// keymaps go from first row to last row in each layer
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE_LAYER] = LAYOUT_split_3x5_2(
        KC_Q, KC_W, KC_F, KC_P, KC_B, KC_J, KC_L, KC_U, KC_Y, QK_REP,
        LCTL_T(KC_A), LGUI_T(KC_R), LALT_T(KC_S), KC_T, KC_G, KC_M, KC_N, RALT_T(KC_E), RGUI_T(KC_I), RCTL_T(KC_O),
        LT(SYS_LAYER,KC_Z), MT(KC_CUT,KC_X), MT(KC_COPY,KC_C), KC_D, MT(KC_PASTE,KC_V), KC_K, KC_H, KC_COMM, KC_DOT, KC_SLSH,
        OSM(MOD_LSFT), KC_SPC, TD(CTL_BSPC), LT(NUM_LAYER,KC_ENT)),
    [SYM_LAYER] = LAYOUT_split_3x5_2(
        KC_NO, KC_AMPR, KC_HASH, KC_DLR, KC_ASTR, KC_GRV, KC_PMNS, KC_LBRC, KC_RBRC, KC_NO,
        KC_COLN, KC_TILD, KC_UNDS, KC_PEQL, KC_PLUS, KC_PIPE, KC_DQUO, KC_LPRN, KC_RPRN, KC_SCLN,
        KC_NO, KC_EXLM, KC_CIRC, KC_PERC, KC_NO, KC_NO, KC_AT, KC_LCBR, KC_RCBR, KC_NO,
        TO(NUM_LAYER), KC_SPC, TD(CTL_BSPC), LT(FNV_LAYER,KC_ENT)),
    [NUM_LAYER] = LAYOUT_split_3x5_2(
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_P4, KC_P5, KC_P6, KC_NO,
        LCTL(KC_TAB), KC_LCTL, KC_LGUI, KC_LALT, ALT_TAB, KC_NO, KC_P1, KC_P2, KC_P3, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_P0, KC_P7, KC_P8, KC_P9, KC_NO,
        TO(SYM_LAYER), KC_SPC, TD(CTL_BSPC), LT(FNV_LAYER,KC_ENT)),
    [FNV_LAYER] = LAYOUT_split_3x5_2(
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F6, KC_NO, LSG(KC_S), KC_NO, KC_NO, KC_NO,
        KC_NO, KC_F5, KC_NO, KC_F11, KC_NO, KC_NO, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT,
        KC_F7, KC_F8, KC_F9, KC_F10, KC_F12, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        TO(SYM_LAYER), KC_SPC, KC_ENT, TO(NUM_LAYER)),
    [SYS_LAYER] = LAYOUT_split_3x5_2(
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        HIBERNATE, LAG(KC_S), LAG(KC_R), LAG(KC_O), KC_NO, LALT(KC_F4), LGUI(KC_C), LAG(KC_C), KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO)
};


// Super Alt-Tab
// Registers Alt-Tab for 1000ms, if pressed again will send another tab if no tap, alt unregistered
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) { // This will do most of the grunt work with the keycodes.
        case ALT_TAB:
            if (record->event.pressed) {
                if (!is_alt_tab_active) {
                    is_alt_tab_active = true;
                    register_code(KC_LALT);
                }
                alt_tab_timer = timer_read();
                register_code(KC_TAB);
            } else {
                unregister_code(KC_TAB);
            }
            break;
        case HIBERNATE:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_X);
                _delay_ms(250);
                unregister_code(KC_LGUI);
                unregister_code(KC_X);
                register_code(KC_U);
                register_code(KC_H);
            } else {
                unregister_code(KC_U);
                unregister_code(KC_H);
            }
            break;
    }
    return true;
}
void matrix_scan_user(void) { // The very important timer.
    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1000) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
}

// Backspace on Single Tap, Ctrl + Backspace on Double Tap + Hold
// Determine tap state
td_state_t cur_dance(tap_dance_state_t *state) {
    if ( state->count == 1 ) {
        if ( state->interrupted || !state->pressed ) return TD_SINGLE_TAP;
        else return TD_UNKNOWN;
    } else if ( state->count == 2 ) {
        if ( state->pressed ) return TD_DOUBLE_HOLD;
        else return TD_UNKNOWN;
    } else return TD_UNKNOWN;
}

// Create instance of td_tap_t for cbs
static td_tap_t cbs_state = {
    .is_press_action = true,
    .state = TD_UNKNOWN
};

// Handle tap states
void cbs_finished(tap_dance_state_t *state, void *user_data) {
    cbs_state.state = cur_dance(state);
    switch (cbs_state.state) {
        case TD_SINGLE_TAP:
            register_code16(KC_BSPC);
            break;
        case TD_DOUBLE_HOLD:
            register_code16(LCTL(KC_BSPC));
            break;
        default:
            break;
    }
}
void cbs_reset(tap_dance_state_t *state, void *user_data) {
    switch (cbs_state.state) {
        case TD_SINGLE_TAP:
            unregister_code16(KC_BSPC);
            break;
        case TD_DOUBLE_HOLD:
            unregister_code16(LCTL(KC_BSPC));
            break;
        default:
            break;
    }
    cbs_state.state = TD_UNKNOWN;
}

tap_dance_action_t tap_dance_actions[] = {
    [CTL_BSPC] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, cbs_finished, cbs_reset)
};
