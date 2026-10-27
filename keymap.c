#include QMK_KEYBOARD_H
#include "definitions.h"
#include "oled.c"
#include "rgb.c"

// trilayer
layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _SYMBOL);
}

// combos
// esc       | tab     | q |   |   |   |   |__|   |   |   |   |   |     |
//           |         |   |   |   |   |   |__|   |   |   |   |   |     |
//           |         |   |   |   |   |   |__|   |   |   |   |   |     |
//           |             |       |       |__|       |       |         |

//           |        |   |   |   |   |   |__|   |   |   |   |   |     |
// caps lock | lshift | a |   |   |   |   |__|   |   |   |   |   |     |
//           |        |   |   |   |   |   |__|   |   |   |   |   |     |
//           |            |       |       |__|       |       |         |

//           |        |   |   |   |   |   |__|   |   |   |   |   |     |
//           |        |   |   |   |   |   |__|   |   |   |   |   |     |
// left gui  | lcontr | z |   |   |   |   |__|   |   |   |   |   |     |
//           |            |       |       |__|       |       |         |

enum combos {
	TABQ_ESC,
    LSFTA_CAPS,
    LCTLZ_LGUI,
    COMBO_LENGTH
};

uint16_t COMBO_LEN = COMBO_LENGTH;

const uint16_t PROGMEM tab_q_combo[] = {KC_TAB, KC_Q, COMBO_END}; // combo 1
const uint16_t PROGMEM lsft_a_combo[] = {KC_LSFT, KC_A, COMBO_END}; // combo 2
const uint16_t PROGMEM lctrl_z_combo[] = {KC_LCTL, KC_Z, COMBO_END}; // combo 3

combo_t key_combos[] = {
    [TABQ_ESC] = COMBO(tab_q_combo, KC_ESC), // combo output 1
	[LSFTA_CAPS] = COMBO(lsft_a_combo, KC_CAPS), // combo output 2
	[LCTLZ_LGUI] = COMBO(lctrl_z_combo, KC_LGUI), // combo output 3
};

// keymaps
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_split_3x6_3(
//                                    |    tab | q | w | e | r | t |  | y | u | i | o | p | bksp |
//                                    | lshift | a | s | d | f | g |  | h | j | k | l | ñ | del  |
//                                    | lcontr | z | x | c | v | b |  | n | m | , | . | - | ins  |
//                                    |   lalt | mo(lower) | spce  |  | enter | mo(raise) |contex|
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       KC_TAB,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                         KC_Y,    KC_U,    KC_I,    KC_O,   KC_P,  KC_BSPC,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_LSFT,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                         KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_DEL,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_LCTL,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                         KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_INS,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                        KC_LALT, MO(_LOWER), KC_SPC,     KC_ENT, MO(_RAISE), KC_APP
                                      //'--------------------------'  `--------------------------'
  ),

    [_LOWER] = LAYOUT_split_3x6_3(
//              |__________|___| mouse1 | mswhl↑ | mswhl↓ | mouse2 |  |  7  |  8  |  9  |  0  |  ^  |  ´  |
//              | capslock |___| mouse← | mouse↑ | mouse↓ | mouse→ |  |  4  |  5  |  6  |_____|  ¨  |  '  |
//              |__________|___|________|________|________|________|  |  1  |  2  |  3  |_____|_____|  `  |
//                             |________|      press      |________|  |_____|_____|_____|
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      XXXXXXX, XXXXXXX, MS_BTN1, MS_WHLU, MS_WHLD, MS_BTN2,                         KC_7,    KC_8,    KC_9,    KC_0, S(KC_LBRC), KC_QUOT,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_CAPS, XXXXXXX, MS_LEFT, MS_UP, MS_DOWN, MS_RGHT,                           KC_4,    KC_5,    KC_6, XXXXXXX, S(KC_QUOT), KC_MINS,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                         KC_1,    KC_2,    KC_3, XXXXXXX, XXXXXXX, KC_LBRC,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          XXXXXXX, _______, XXXXXXX,    XXXXXXX, _______, XXXXXXX
                                      //'--------------------------'  `--------------------------'
  ),

    [_RAISE] = LAYOUT_split_3x6_3(
//                         | rgbtoggle |____|____|_____|_____|_____|  | home | pgup | pgdn | end |____|_____|
//                         |___________|____|____|_____|_____|_____|  |   ←  |   ↑  |   ↓  |  →  |____|_____|
//                                | f7 | f8 | f9 | f10 | f11 | f12 |  |  f1  |  f2  |  f3  |  f4 | f5 |  f6 |
//                                               |_____|_____|_____|  |______|    press    |__________|
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      RM_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                      KC_HOME, KC_PGUP, KC_PGDN,  KC_END, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                      KC_LEFT,   KC_UP, KC_DOWN, KC_RGHT, XXXXXXX, XXXXXXX,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
        KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12,                        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          XXXXXXX, _______, XXXXXXX,    XXXXXXX, _______, XXXXXXX
                                      //'--------------------------'  `--------------------------'
  ),

    [_SYMBOL] = LAYOUT_split_3x6_3(
//                          |     \  |  !  |  "  |  ·  |  $  |  %  |  |  &  |  /  |  (  |  )  |  =  |  '  |
//                          |  caps  |  |  |  @  |  #  |  ~  |  €  |  |  ¬  |  |  |  [  |  ]  |  ^  |  ¡  |
//                          |     <  |  >  |  +  |  -  |  *  |  /  |  |  ^  |  \  |  {  |  }  |_____|  º  |
//                                   |___________|   press   |_____|  |_____|   press   |___________|
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      RALT(KC_GRV), S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),                 S(KC_6), S(KC_7), S(KC_8), S(KC_9), S(KC_0), KC_MINS,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_LSFT, RALT(KC_1), RALT(KC_2), RALT(KC_3), RALT(KC_4), RALT(KC_5),    RALT(KC_6),RALT(KC_1),RALT(KC_LBRC),RALT(KC_RBRC),S(KC_LBRC),KC_EQL,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_NUBS, S(KC_NUBS), KC_PPLS, KC_PMNS, KC_PAST, KC_PSLS,                S(KC_LBRC),RALT(KC_GRV),RALT(KC_QUOT),RALT(KC_NUHS),XXXXXXX,KC_GRV,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          XXXXXXX, _______, XXXXXXX,    XXXXXXX, _______, XXXXXXX
                                      //'--------------------------'  `--------------------------'
  )
};

// interactive callbacks
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    semiotic_process_record(keycode, record); // oled.c
    rgb_process_record(keycode, record);      // rgb.c
    return true;
}
