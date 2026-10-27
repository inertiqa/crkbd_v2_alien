#include QMK_KEYBOARD_H
#include "definitions.h"

#ifdef OLED_ENABLE

#include "graphics/w_y_old.c" // slave glitch graphics
// #include "graphics/seegson.c"
// #include "graphics/w_y_new.c"
// #include "graphics/arasaka.c"

#define OLED_WIDTH               128
#define OLED_HEIGHT              32
#define OLED_PAGES               (OLED_HEIGHT / 8)
#define FRAME_SIZE               512

#define GLITCH_FRAME_INTERVAL_MS 33
#define GLITCH_TRIGGER_CHANCE    150
#define GLITCH_DURATION_MIN_MS   250
#define GLITCH_DURATION_RAND_MS  500
#define GLITCH_MAX_BURSTS        9
#define GLITCH_MAX_SHIFT_PX      14

static char glitch_buffer[FRAME_SIZE];
static uint32_t anim_timer = 0;
static uint32_t glitch_end_time = 0;
static bool is_glitching = false;
static bool frame_is_clean = false; // last drawn frame, unglitched

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    oled_clear();
    return OLED_ROTATION_270;
}

static void draw_semiotic_block(uint8_t block) {
    oled_set_cursor(0, block * 4);
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 5; x++) {
            oled_write_char(32 + y * 32 + block * 5 + x, false);
        }
    }
}

// icon invert on keypress
#define SEMIOTIC_ICON_COUNT 16
#define ICON_W              13
#define ICON_RIGHT_X        17   // right icono starts at row byte 17
#define OLED_ROW_BYTES      32   // rotation 90/270: 32 bytes per page

static uint16_t semiotic_active = 0;    // bit i = sticky change
static bool     semiotic_dirty  = true; // redraw start

typedef struct {
    uint16_t keycode;
    uint8_t  icon;    // 0..15 = bloque*4 + mitad*2 + lado
    bool     toggle;  // false = invertido mientras se mantiene pulsada
} semiotic_bind_t;

static const semiotic_bind_t semiotic_binds[] = {
    { KC_A,    0, false },
    { KC_LCTL, 1, false },
    { KC_X,    2, false },
    { KC_W,    3, false },

    { KC_F2,    4, false },
    { KC_CAPS, 5, true  },
    { KC_0,    6, false },
    { KC_E,    7, false },

    { KC_7,    8, false },
    { KC_I,    9, false },
    { KC_K,    10, false },
    { KC_U,    11, false },

    { KC_O,    12, false },
    { KC_LSFT, 13, false },
    { KC_X,    14, false },
    { KC_9,    15, false },
};

static void semiotic_invert_icon(uint8_t icon) {
    uint8_t block = icon / 4;
    uint8_t half  = (icon / 2) % 2;
    uint8_t side  = icon % 2;
    uint8_t page  = block * 4 + half * 2;
    uint8_t x0    = side ? ICON_RIGHT_X : 0;

    //incons bitmasks
    for (uint8_t c = 1; c < ICON_W - 1; c++) { // 0 and 12th columns
        uint8_t top_mask, bot_mask;
        if (c == 1 || c == ICON_W - 2) { // rounded edges
            top_mask = 0xF8; bot_mask = 0x0F;
        } else {                         // top and bottom border
            top_mask = 0xFC; bot_mask = 0x1F;
        }
        uint16_t it = page * OLED_ROW_BYTES + x0 + c;
        uint16_t ib = it + OLED_ROW_BYTES;
        // oled_read_raw() returns oled_buffer_reader_t; byte in current_element
        oled_write_raw_byte(*oled_read_raw(it).current_element ^ top_mask, it);
        oled_write_raw_byte(*oled_read_raw(ib).current_element ^ bot_mask, ib);
    }
}

// xor invert: only for icons changed since last redraw
static uint16_t semiotic_drawn       = 0;
static bool     semiotic_initialized = false;

static void semiotic_update(void) {
    if (!semiotic_initialized) {
        for (uint8_t b = 0; b < 4; b++) draw_semiotic_block(b);
        semiotic_drawn       = 0;
        semiotic_initialized = true;
    }
    uint16_t diff = semiotic_active ^ semiotic_drawn;
    for (uint8_t i = 0; i < SEMIOTIC_ICON_COUNT; i++) {
        if (diff & ((uint16_t)1 << i)) semiotic_invert_icon(i);
    }
    semiotic_drawn = semiotic_active;
}

// call from process_record_user (master side only)
void semiotic_process_record(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; i < ARRAY_SIZE(semiotic_binds); i++) {
        if (semiotic_binds[i].keycode != keycode) continue;
        uint16_t bit = (uint16_t)1 << semiotic_binds[i].icon;
        uint16_t old = semiotic_active;
        if (semiotic_binds[i].toggle) {
            if (record->event.pressed) semiotic_active ^= bit;
        } else {
            if (record->event.pressed) semiotic_active |= bit;
            else                       semiotic_active &= ~bit;
        }
        if (old != semiotic_active) semiotic_dirty = true;
    }
}

// glitching
static void render_slave_glitch(void) {
    if (timer_elapsed32(anim_timer) <= GLITCH_FRAME_INTERVAL_MS) return;
    uint32_t now = timer_read32();
    anim_timer = now;

    // wait for delta to redraw
    if (is_glitching && timer_expired32(now, glitch_end_time)) {
        is_glitching = false;
    }
    if (!is_glitching && (rand() % GLITCH_TRIGGER_CHANCE == 0)) {
        is_glitching    = true;
        glitch_end_time = now + GLITCH_DURATION_MIN_MS + (rand() % (GLITCH_DURATION_RAND_MS + 1));
    }

    // no glitch and base frame = return
    if (!is_glitching && frame_is_clean) return;

    memcpy_P(glitch_buffer, w_y_old, FRAME_SIZE);

    if (is_glitching) {
        uint8_t bursts = (rand() % GLITCH_MAX_BURSTS) + 1;
        for (uint8_t b = 0; b < bursts; b++) {
            uint16_t start = rand() % (FRAME_SIZE - 12);
            uint8_t  len   = (rand() % 8) + 3;
            for (uint8_t i = 0; i < len; i++) {
                glitch_buffer[start + i] ^= (char)(rand() & 0xFF);
            }
        }

        // 8px page shift
        uint8_t glitch_page = rand() % OLED_PAGES;
        int8_t  shift       = (rand() % ((GLITCH_MAX_SHIFT_PX * 2) + 1)) - GLITCH_MAX_SHIFT_PX;

        char *page_ptr = &glitch_buffer[glitch_page * OLED_WIDTH];
        char  temp_page[OLED_WIDTH];
        memcpy(temp_page, page_ptr, OLED_WIDTH);

        for (int x = 0; x < OLED_WIDTH; x++) {
            int src_x = x - shift;
            page_ptr[x] = (src_x >= 0 && src_x < OLED_WIDTH) ? temp_page[src_x] : 0x00;
        }
    }

    frame_is_clean = !is_glitching;
    oled_write_raw(glitch_buffer, FRAME_SIZE);
}

// render loop
bool oled_task_user(void) {
    if (is_keyboard_master()) {
        if (semiotic_dirty) {
            semiotic_dirty = false;
            semiotic_update();
        }
    } else {
        render_slave_glitch();
    }
    return false;
}

#endif // OLED_ENABLE

