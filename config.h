#pragma once

#define EE_HANDS // both sides can be master
#define SPLIT_LAYER_STATE_ENABLE // layer changes synchro sides
#define SPLIT_LED_STATE_ENABLE // led changes and caps state synchro sides
#define SPLIT_TRANSPORT_MIRROR // rgb matrix led changes synchro sides on the trrs bus
#define SPLIT_TRANSACTION_IDS_USER KEYBOARD_SYNC_RGB

#define RGB_DI_PIN D1 // for corne v2 pcb
#undef RGB_MATRIX_MAXIMUM_BRIGHTNESS
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 127 // limit led voltage so controllers doesnt fry like a gtx4090

#define RGB_MATRIX_LED_COUNT 54

#undef RGB_MATRIX_SPLIT
#define RGB_MATRIX_SPLIT { 27, 27 }

// oled
#define OLED_FONT_H "keyboards/crkbd/keymaps/inertiqa/glcdfont.c"
#define SPLIT_OLED_ENABLE // slave side graphic animation stops when timeout on master side fires
#define OLED_TIMEOUT 60000 // oled screen blackout timeout if away (ms - 1 min)
#define RGB_MATRIX_TIMEOUT 300000 // led blackout timeout if away (ms - 5 min)

#define TAPPING_TERM 175 // tapdance window (ms)
#define COMBO_TERM 40 // combo window (ms)

