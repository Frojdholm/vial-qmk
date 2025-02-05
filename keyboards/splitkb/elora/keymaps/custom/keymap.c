/* Copyright 2023 splitkb.com <support@splitkb.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "send_string_keycodes.h"
#include QMK_KEYBOARD_H

#include "quantum_keycodes.h"
#include "report.h"
#include "keycodes.h"

enum layers {
    _QWERTY = 0,
    _COLEMAK_DH,
    _MOUSE,
    _NAV,
    _SYM,
    _ADJUST,
    _EMPTY,
};

enum custom_keycodes {
    SCROLL_MODE = SAFE_RANGE,
    UPDIR,
    HOMEDIR,
    // Non-dead versions of keys
    NDHAT,
    NDTILDE,
    NDACC,
    NDSACC,
};

// Aliases for readability
#define QWERTY   DF(_QWERTY)
#define COLEMAK  DF(_COLEMAK_DH)

#define SYM      OSL(_SYM)
#define NAV      LT(_NAV, KC_ENT)
#define ADJUST   LT(_ADJUST, KC_CAPS)
#define MOUSE    TG(_MOUSE)

#define CTL_QUOT MT(MOD_RCTL, KC_QUOTE)
#define ALT_ENT  MT(MOD_LALT, KC_ENT)
#define ALT_SPC  MT(MOD_RALT, KC_SPC)

// Note: LAlt/Enter (ALT_ENT) is not the same thing as the keyboard shortcut Alt+Enter.
// The notation `mod/tap` denotes a key that activates the modifier `mod` when held down, and
// produces the key `tap` when tapped (i.e. pressed and released).

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_myr(
      KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,         KC_LSFT,     KC_RSFT,          KC_6 ,  KC_7 ,  KC_8 ,   KC_9 ,  KC_0 , KC_BSPC,
      KC_TAB  , KC_Q ,  KC_W   ,  KC_E  ,   KC_R ,   KC_T ,         KC_LCTL,     KC_RCTL,          KC_Y ,  KC_U ,  KC_I ,   KC_O ,  KC_P , KC_LBRC,
      KC_LCTL, KC_A ,  KC_S   ,  KC_D  ,   KC_F ,   KC_G ,         KC_LALT,     KC_RALT,          KC_H ,  KC_J ,  KC_K ,   KC_L ,KC_SCLN,CTL_QUOT,
      KC_LSFT , KC_Z ,  KC_X   ,  KC_C  ,   KC_V ,   KC_B , ADJUST , SCROLL_MODE ,    KC_BTN1  , KC_BTN2, KC_N ,  KC_M ,KC_COMM, KC_DOT ,KC_SLSH, KC_RSFT,
                                 ADJUST , KC_LGUI, ALT_SPC, NAV , SYM   ,     SYM    , KC_SPC , ALT_ENT, KC_RGUI, KC_DEL,

      KC_MPRV, KC_MNXT, KC_MPLY, KC_MSTP,    KC_MUTE,                            KC_MPRV, KC_MNXT, KC_MPLY, KC_MSTP,    KC_MUTE
    ),
    [_COLEMAK_DH] = LAYOUT_myr(
      KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,         KC_LSFT,     KC_RSFT,          KC_6 ,  KC_7 ,  KC_8 ,   KC_9 ,  KC_0 , KC_BSPC,
      KC_TAB  , KC_Q ,  KC_W   ,  KC_F  ,   KC_P ,   KC_B ,         KC_LCTL,     KC_RCTL,          KC_J,   KC_L ,  KC_U ,   KC_Y ,KC_SCLN, KC_LBRC,
      KC_LCTL, KC_A ,  KC_R   ,  KC_S  ,   KC_T ,   KC_G ,         KC_LALT,     KC_RALT,          KC_M,   KC_N ,  KC_E ,   KC_I ,  KC_O , CTL_QUOT,
      KC_LSFT , KC_Z ,  KC_X   ,  KC_C  ,   KC_D ,   KC_V , ADJUST , SCROLL_MODE ,     KC_BTN1  , KC_BTN2, KC_K,   KC_H ,KC_COMM, KC_DOT ,KC_SLSH, KC_RSFT,
                                 ADJUST , KC_LGUI, ALT_SPC, NAV , SYM   ,     SYM    , KC_SPC , ALT_ENT, KC_RGUI, KC_DEL,

      KC_MPRV, KC_MNXT, KC_MPLY, KC_MSTP,    KC_MUTE,                            KC_MPRV, KC_MNXT, KC_MPLY, KC_MSTP,    KC_MUTE
    ),
    [_MOUSE] = LAYOUT_myr(
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______,SCROLL_MODE, _______, _______,          _______, _______,          _______, _______, KC_MS_UP,   _______,  _______, _______,
      _______, KC_BTN4, KC_BTN5, KC_BTN1, KC_BTN2, KC_BTN3,          _______, _______,          _______, KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT, _______, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,_______, KC_MS_WH_DOWN, KC_MS_WH_UP, _______, _______, _______,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
    ),
    [_NAV] = LAYOUT_myr(
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,          _______, _______,          KC_PGUP, KC_HOME, KC_UP,   KC_END,  KC_VOLU, KC_DEL,
      _______, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______,          _______, _______,          KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_VOLD, KC_INS,
      _______, _______, _______, _______, _______, _______, _______, KC_SCRL, _______, _______,KC_PAUSE, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_PSCR,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
    ),
    [_SYM] = LAYOUT_myr(
      _______,   KC_F1 ,  KC_F2 ,  KC_F3 ,  KC_F4 ,  KC_F5 ,          _______, _______,          KC_F6  ,  KC_F7 ,  KC_F8 ,  KC_F9 , KC_F10 , _______,
      _______, KC_NUHS, KC_NUBS,S(KC_NUBS),S(KC_2), NDSACC,          _______, _______,          S(KC_EQL),S(KC_6),RALT(KC_8),RALT(KC_9),S(KC_5), KC_F11,
      _______, S(KC_1),KC_SLASH,KC_MINUS, S(KC_0), S(KC_3),          _______, _______,          KC_EQL,NDTILDE,S(KC_8),S(KC_9),S(KC_MINUS), _______,
      _______,NDHAT,S(KC_7),S(KC_NUHS),RALT(KC_MINUS),NDACC,_______, _______, _______,_______,RALT(KC_NUBS),RALT(KC_4),RALT(KC_7),RALT(KC_0),RALT(KC_2),_______,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
    ),
    [_ADJUST] = LAYOUT_myr(
      _______, _______, _______, _______, _______, _______,         _______, _______,          _______, _______, _______, _______,  _______, _______,
      _______, _______, _______, QWERTY , _______, _______,         _______, _______,          _______, _______, _______, _______,  _______, _______,
      _______, _______, _______, COLEMAK, _______, _______,         _______, _______,          RGB_TOG, RGB_SAI, RGB_HUI, RGB_VAI,  RGB_MOD, _______,
      _______, _______, _______, _______, _______, _______,_______, _______, _______, _______, _______, RGB_SAD, RGB_HUD, RGB_VAD, RGB_RMOD, _______,
                                 _______, _______, _______,_______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______

    ),
    [_EMPTY] = LAYOUT_myr(
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
    ),

// /*
//  * Layer template - LAYOUT
//  *
//  * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        `----------------------------------'  `----------------------------------'
//  */
//     [_LAYERINDEX] = LAYOUT(
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
//     ),

// /*
//  * Layer template - LAYOUT_myr
//  *
//  * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        `----------------------------------'  `----------------------------------'
//  *
//  * ,-----------------------------.      ,------.                ,---------------------------.      ,------.
//  * |        |      |      |      |      |      |                |      |      |      |      |      |      |
//  * `-----------------------------'      `------'                `---------------------------'      '------'
//  */
//     [_LAYERINDEX] = LAYOUT_myr(
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//
//       _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
//     ),
};

/* The default OLED and rotary encoder code can be found at the bottom of qmk_firmware/keyboards/splitkb/elora/rev1/rev1.c
 * These default settings can be overriden by your own settings in your keymap.c
 * DO NOT edit the rev1.c file; instead override the weakly defined default functions by your own.
 */

/* DELETE THIS LINE TO UNCOMMENT (1/2)
#ifdef OLED_ENABLE
bool oled_task_user(void) {
  // Your code goes here
}
#endif

#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
  // Your code goes here
}
#endif
DELETE THIS LINE TO UNCOMMENT (2/2) */

static bool scrolling_mode = false;
static uint16_t myr_scroll_timer_left = 0;
static uint16_t myr_scroll_timer_right = 0;
static const uint16_t scroll_base_delay = 32 * 20;

static mouse_xy_report_t signum_mouse_report(mouse_xy_report_t val) {
    return (val > 0) - (val < 0);
}

static mouse_xy_report_t max_mouse_report(mouse_xy_report_t a, mouse_xy_report_t b) {
    return a > b ? a : b;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCROLL_MODE:
            if (record->event.pressed) {
                scrolling_mode = true;
            } else { // released
                scrolling_mode = false;
            }
            return false;
        case UPDIR:
            if (record->event.pressed) {
                // Macro on Swedish layout: ../
                SEND_STRING(".." SS_LSFT("7"));
            } else { // released
                // nop
            }
            break;
        case HOMEDIR:
            if (record->event.pressed) {
                // Macro on Swedish layout (with dead-keys): ~/
                SEND_STRING(SS_ALGR("]") SS_LSFT("7"));
            } else { // released
                // nop
            }
            break;
        case NDHAT:
            if (record->event.pressed) {
                // Macro on Swedish layout (with dead-keys): ^
                SEND_STRING(SS_LSFT("] "));
            } else { // released
                // nop
            }
            break;
        case NDTILDE:
            if (record->event.pressed) {
                // Macro on Swedish layout (with dead-keys): ~
                SEND_STRING(SS_ALGR("]") " ");
            } else { // released
                // nop
            }
            break;
        case NDACC:
            if (record->event.pressed) {
                // Macro on Swedish layout (with dead-keys): ´
                SEND_STRING("= ");
            } else { // released
                // nop
            }
            break;
        case NDSACC:
            if (record->event.pressed) {
                // Macro on Swedish layout (with dead-keys): `
                SEND_STRING(SS_LSFT("= "));
            } else { // released
                // nop
            }
            break;

    }
    return true;
}

bool is_mouse_record_user(uint16_t keycode, keyrecord_t *record) {
    return keycode == SCROLL_MODE;
}

report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    if (scrolling_mode) {
        const mouse_xy_report_t left_mult = max_mouse_report(abs(left_report.y), abs(left_report.x));
        if (left_mult * timer_elapsed(myr_scroll_timer_left) > scroll_base_delay) {
            left_report.v = -signum_mouse_report(left_report.y);
            left_report.h = signum_mouse_report(left_report.x);
            myr_scroll_timer_left = timer_read();
        }

        const mouse_xy_report_t right_mult = max_mouse_report(abs(right_report.y), abs(right_report.x));
        if (right_mult * timer_elapsed(myr_scroll_timer_right) > scroll_base_delay) {
            right_report.v = -signum_mouse_report(right_report.y);
            right_report.h = signum_mouse_report(right_report.x);
            myr_scroll_timer_right = timer_read();
        }

        left_report.x = 0;
        left_report.y = 0;
        right_report.x = 0;
        right_report.y = 0;
    }
    return pointing_device_combine_reports(left_report, right_report);
}
