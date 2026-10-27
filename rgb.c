#include QMK_KEYBOARD_H
#include "definitions.h"
#include "transactions.h"

#ifdef RGB_MATRIX_ENABLE

#define MAINFRAME_INTERVAL_MS 1666  // pattern change
#define MAINFRAME_CHANCE      4     // 1 in 4 chance of lit led
#define MAINFRAME_BRIGHTNESS  255   // scaled by RGB_MATRIX_MAXIMUM_BRIGHTNESS
#define REACTIVE_BRIGHTNESS   255   // keypress brightness, same as MAINFRAME_BRIGHTNESS

#define REACTIVE_DECAY_STEP   4     // light dimmer per frame
#define REACTIVE_FRAME_MS     16    // ~60 Hz

#define COLOR_MAIN_R   155
#define COLOR_MAIN_G   255
#define COLOR_MAIN_B   255

#define COLOR_CAPS_R   127
#define COLOR_CAPS_G   127
#define COLOR_CAPS_B   127

// caps lock indicator (both sides)
#define CAPS_LEFT_FIRST   24
#define CAPS_LEFT_LAST    26
#define CAPS_RIGHT_FIRST  51
#define CAPS_RIGHT_LAST   53

static bool    led_active_states[RGB_MATRIX_LED_COUNT];
static uint8_t led_fade_values[RGB_MATRIX_LED_COUNT] = {0};

static uint32_t mainframe_timer = 0;
static uint32_t decay_timer     = 0;

// trrs sync master -> slave

// custom packet: bitmask pattern + last keypress index
// RPC_M2S_BUFFER_SIZE limit 32 bytes, unable to send full status
#define PATTERN_BYTES ((RGB_MATRIX_LED_COUNT + 7) / 8)
#define NO_PRESS      0xFF

typedef struct {
    uint8_t pattern[PATTERN_BYTES];
    uint8_t pressed; // led keypress index
} rgb_sync_t;

_Static_assert(sizeof(rgb_sync_t) <= 32, "rgb_sync_t exceeds default RPC_M2S_BUFFER_SIZE");

// master: sends packet (+ keypress if any)
static void sync_rgb_to_slave(uint8_t pressed) {
    if (!is_keyboard_master()) return;

    rgb_sync_t data = { .pressed = pressed };
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (led_active_states[i]) data.pattern[i >> 3] |= (1 << (i & 7));
    }
    transaction_rpc_send(KEYBOARD_SYNC_RGB, sizeof(data), &data);
}

// slave: gets packet and updates
static void slave_rgb_sync_handler(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len != sizeof(rgb_sync_t)) return;
    const rgb_sync_t *d = (const rgb_sync_t *)in_data;

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        led_active_states[i] = (d->pattern[i >> 3] >> (i & 7)) & 1;
    }
    if (d->pressed < RGB_MATRIX_LED_COUNT) {
        led_fade_values[d->pressed]   = 255;
        led_active_states[d->pressed] = false;
    }
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(KEYBOARD_SYNC_RGB, slave_rgb_sync_handler);

    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(0, 0, 0);
}

void suspend_power_down_user(void) {
    rgb_matrix_set_suspend_state(true);
}

void suspend_wakeup_init_user(void) {
    rgb_matrix_set_suspend_state(false);
}

static void generate_mainframe_pattern(void) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        led_active_states[i] = (led_fade_values[i] == 0) && ((rand() % MAINFRAME_CHANCE) == 0);
    }
}

void matrix_scan_user(void) {
    uint32_t now = timer_read32();

    // dimmer is calculated at both sides to save on bus transfers
    if (timer_elapsed32(decay_timer) >= REACTIVE_FRAME_MS) {
        decay_timer = now;
        for (uint8_t j = 0; j < RGB_MATRIX_LED_COUNT; j++) {
            if (led_fade_values[j] > REACTIVE_DECAY_STEP) {
                led_fade_values[j] -= REACTIVE_DECAY_STEP;
            } else {
                led_fade_values[j] = 0;
            }
        }
    }

    // new key pattern, sent to slave
    if (is_keyboard_master() && timer_elapsed32(mainframe_timer) >= MAINFRAME_INTERVAL_MS) {
        mainframe_timer = now;
        generate_mainframe_pattern();
        sync_rgb_to_slave(NO_PRESS);
    }
}

// calling from process_record_user (keymap.c) - only on master
void rgb_process_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return;

    uint8_t led_index = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
    if (led_index != NO_LED && led_index < RGB_MATRIX_LED_COUNT) {
        led_fade_values[led_index]   = 255;
        led_active_states[led_index] = false;
        sync_rgb_to_slave(led_index);
    }
}

// RGB_MATRIX_MAXIMUM_BRIGHTNESS light scaling - rgb_matrix_set_color() ignores the definition
static void set_scaled_led_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    uint16_t level = (uint16_t)brightness * RGB_MATRIX_MAXIMUM_BRIGHTNESS / 255;
    rgb_matrix_set_color(index,
                         (uint16_t)r * level / 255,
                         (uint16_t)g * level / 255,
                         (uint16_t)b * level / 255);
}

// both sides manages their own led states
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    bool caps_active = host_keyboard_led_state().caps_lock;

    for (uint8_t i = led_min; i < led_max; i++) {
        if (caps_active && ((i >= CAPS_LEFT_FIRST && i <= CAPS_LEFT_LAST) ||
                            (i >= CAPS_RIGHT_FIRST && i <= CAPS_RIGHT_LAST))) {
            rgb_matrix_set_color(i, COLOR_CAPS_R, COLOR_CAPS_G, COLOR_CAPS_B);
            continue;
        }

        if (led_fade_values[i] > 0) {
            set_scaled_led_color(i, COLOR_MAIN_R, COLOR_MAIN_G, COLOR_MAIN_B,
                                 (uint16_t)led_fade_values[i] * REACTIVE_BRIGHTNESS / 255);
        } else if (led_active_states[i]) {
            set_scaled_led_color(i, COLOR_MAIN_R, COLOR_MAIN_G, COLOR_MAIN_B, MAINFRAME_BRIGHTNESS);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }

    return false;
}

#endif // RGB_MATRIX_ENABLE
