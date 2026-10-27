# hardware specifics
LTO_ENABLE = yes # link-time optimizations - mandatory on low memory micros
WEAR_LEVELING_DRIVER = rp2040_flash # internal memory for dynamic layers
EEPROM_DRIVER = wear_leveling # mandatory for rp2040, eeprom flash emulation

# corne specifics
SPLIT_KEYBOARD = yes

# screens and lighting
VIALRGB_ENABLE = no
OLED_ENABLE = yes
RGBLIGHT_ENABLE = no
RGB_MATRIX_ENABLE = yes # per-key lighting
WS2812_DRIVER = vendor

# entry, functions, macros
AUTO_SHIFT_ENABLE = no # hold a key sends caps version
BOOTMAGIC_ENABLE = no # bootloader holding a key
CAPS_WORD_ENABLE = yes # caps lock until press space or symbol
COMMAND_ENABLE = no # debug console
CONSOLE_ENABLE = no # more debug console
VIA_ENABLE = yes # required by vial
VIAL_ENABLE = yes
VIAL_INSECURE = yes # skip unlock combo; set to no once the unlock combo is verified
DYNAMIC_KEYMAP_ENABLE = yes # for via/vial
DYNAMIC_MACRO_ENABLE = yes # record macros on the fly
EXTRAKEY_ENABLE = yes # multimedia, vol, bright, energy
KEY_OVERRIDE_ENABLE = no # modify specific key presses
LEADER_ENABLE = no # vim-style leader sequences
MOUSEKEY_ENABLE = yes # mouse with keys
NKRO_ENABLE = yes # n-key rollover
SPACE_CADET_ENABLE = no # mod keys with a rapid press
COMBO_ENABLE = yes # multiple press sends arbitrary key
TAP_DANCE_ENABLE = no # multiple meanings for each key if single press, double press or hold
WPM_ENABLE = no # words per minute counter (unused, saves flash memory)
