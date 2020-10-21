// ChordMappings.h
// Chordmappings for 7 button chorder, split out from FeatherChorder.ino
// Mappings moved here so they can be changed without risk
// of modifying the rest of the code.
// - Greg

#include "hid_dev.h"
#include <stdint.h>

typedef uint16_t keymap_t;

enum nonkeys {
  DIV_nonkeys_offset = 256, // 256 puts us past any uint8_t key in hid_dev.h
/* The following are not standard USB HID, but are the modifier keys,
   handeled specially in decoding and mapped to the modifier byte in
   the USB report */
  DIV_Mods,
  MOD_LCTRL=DIV_Mods,   // 0x01
  MOD_LSHIFT,           // 0x02
  MOD_LALT,             // 0x04
  MOD_LGUI,             // 0x08
  MOD_RCTRL,            // 0x10
  MOD_RSHIFT,           // 0x20
  MOD_RALT,             // 0x40
  MOD_RGUI,             // 0x80

/* Next comes the mode change codes */
  DIV_Modes,
  MODE_RESET=DIV_Modes, // Reset (Default mode)
  MODE_MRESET,          // Master Reset and ID
  MODE_NUM,             // Number/symbols mode
  MODE_NUMLCK,          // Number/symbols lock
  MODE_FUNC,            // Function key mode
  MODE_FUNCLCK,         // Function key lock

/* Then a special mode for both num/sym and shift */
  DIV_Multi,
  MULTI_NumShift=DIV_Multi,
  MULTI_CtlAlt,

/* media keys for testing */
  MEDIA_playpause,     // PLAYPAUSE
  MEDIA_next,     // MEDIANEXT
  MEDIA_previous, // MEDIAPREVIOUS
  MEDIA_stop,     // MEDIASTOP
  MEDIA_volup,     // Volume up
  MEDIA_voldn,     // Volume dn

/* And finally macros, that generate multiple key presses */
  DIV_Macro,
  MACRO_000=DIV_Macro,  // 000
  MACRO_00,             // 00
  MACRO_quotes,         // "" and left arrow
  MACRO_parens,         // () and left arrow
  MACRO_dollar,         // aka, FORCE_LSHIFT|KEY_4
  MACRO_percent,        // aka, FORCE_LSHIFT|KEY_5
  MACRO_ampersand,      // aka, FORCE_LSHIFT|KEY_7
  MACRO_asterisk,       // aka, FORCE_LSHIFT|KEY_8
  MACRO_question,       // aka, FORCE_LSHIFT|KEY_slash
  MACRO_plus,           // aka, FORCE_LSHIFT|KEY_equal
  MACRO_openparen,      // aka, FORCE_LSHIFT|KEY_9
  MACRO_closeparen,     // aka, FORCE_LSHIFT|KEY_0
  MACRO_opencurly,      // aka, FORCE_LSHIFT|lbr
  MACRO_closecurly,     // aka, FORCE_LSHIFT|rbr
  ANDROID_search,       // aka, ALT|KEY_spc
  ANDROID_home,         // aka, ALT|KEY_esc
  ANDROID_menu,         // aka, CTRL|KEY_esc
  ANDROID_back,         // aka, KEY_esc with NO MODS
  ANDROID_dpadcenter,   // aka, KEY_KP5 with NO MODS
  DIV_Last
};


/**********************************************************
 *  order is  Far Thumb, Center Thumb, Near Thumb button  *
 *  Index Finger, Middle Finger, Ring Finger, Pinky       *
 *  FCN IMRP                                              *
 **********************************************************/
const keymap_t keymap_default[128] = {
  HID_KEY_RESERVED,                 // --- ----  0x00 no keys pressed
  HID_KEY_W,                        // --- ---P  0x01
  HID_KEY_Y,                        // --- --R-  0x02
  HID_KEY_U,                        // --- --RP  0x03
  HID_KEY_R,                        // --- -M--  0x04
  MACRO_closeparen,                 // --- -M-P  0x05
  HID_KEY_H,                        // --- -MR-  0x06
  HID_KEY_S,                        // --- -MRP  0x07

  HID_KEY_I,                        // --- I---  0x08
  HID_KEY_B,                        // --- I--P  0x09
  HID_KEY_K,                        // --- I-R-  0x0A
  HID_KEY_Z,                        // --- I-RP  0x0B
  HID_KEY_D,                        // --- IM--  0x0C
  MACRO_openparen,                  // --- IM-P  0x0D
  HID_KEY_E,                        // --- IMR-  0x0E
  HID_KEY_T,                        // --- IMRP  0x0F

  MODE_NUM,                         // --N ----  0x10
  MODE_FUNC,                        // --N ---P  0x11
  HID_KEY_ESCAPE,                   // --N --R-  0x12
  HID_KEY_SEMI_COLON,               // --N --RP  0x13
  HID_KEY_COMMA,                    // --N -M--  0x14
  MACRO_closecurly,                 // --N -M-P  0x15
  HID_KEY_DOT,                      // --N -MR-  0x16
  MOD_LALT,                         // --N -MRP  0x17

  HID_KEY_RESERVED,                 // --N I---  0x18
  HID_KEY_INSERT,                   // --N I--P  0x19
  MOD_LGUI,                         // --N I-R-  0x1A
  MOD_LCTRL,                        // --N I-RP  0x1B
  HID_KEY_F9,                       // --N IM--  0x1C
  MACRO_opencurly,                  // --N IM-P  0x1D
  HID_KEY_SGL_QUOTE,                // --N IMR-  0x1E
  HID_KEY_NUM_LOCK,                 // --N IMRP  0x1F

  HID_KEY_SPACEBAR,                 // -C- ----  0x20
  HID_KEY_F,                        // -C- ---P  0x21
  HID_KEY_G,                        // -C- --R-  0x22
  HID_KEY_V,                        // -C- --RP  0x23
  HID_KEY_C,                        // -C- -M--  0x24
  HID_KEY_RIGHT_BRKT,               // -C- -M-P  0x25
  HID_KEY_P,                        // -C- -MR-  0x26
  HID_KEY_N,                        // -C- -MRP  0x27

  HID_KEY_L,                        // -C- I---  0x28
  HID_KEY_X,                        // -C- I--P  0x29
  HID_KEY_J,                        // -C- I-R-  0x2A
  HID_KEY_Q,                        // -C- I-RP  0x2B
  HID_KEY_M,                        // -C- IM--  0x2C
  HID_KEY_LEFT_BRKT,                // -C- IM-P  0x2D
  HID_KEY_A,                        // -C- IMR-  0x2E
  HID_KEY_O,                        // -C- IMRP  0x2F

  MULTI_NumShift,                   // -CN ----  0x30
  HID_KEY_RESERVED,                 // -CN ---P  0x31
  HID_KEY_RESERVED,                 // -CN --R-  0x32
  HID_KEY_RESERVED,                 // -CN --RP  0x33
  ANDROID_dpadcenter,               // -CN -M--  0x34
  HID_KEY_RESERVED,                 // -CN -M-P  0x35
  ANDROID_home,                     // -CN -MR-  0x36
  MOD_RALT,                         // -CN -MRP  0x37

  HID_KEY_RESERVED,                 // -CN I---  0x38
  ANDROID_back,                     // -CN I--P  0x39
  MOD_RGUI,                         // -CN I-R-  0x3A
  MOD_RCTRL,                        // -CN I-RP  0x3B
  ANDROID_menu,                     // -CN IM--  0x3C
  HID_KEY_RESERVED,                 // -CN IM-P  0x3D
  ANDROID_search,                   // -CN IMR-  0x3E
  HID_KEY_NUM_LOCK,                 // -CN IMRP  0x3F

  MOD_LSHIFT,                       // F-- ----  0x40
  HID_KEY_RETURN,                   // F-- ---P  0x41
  HID_KEY_RIGHT_ARROW,              // F-- --R-  0x42
  HID_KEY_DOWN_ARROW,               // F-- --RP  0x43
  HID_KEY_DELETE,                   // F-- -M--  0x44
  HID_KEY_PRNT_SCREEN,              // F-- -M-P  0x45
  HID_KEY_DELETE_FWD,               // F-- -MR-  0x46
  HID_KEY_PAGE_DOWN,                // F-- -MRP  0x47

  HID_KEY_LEFT_ARROW,               // F-- I---  0x48
  HID_KEY_END,                      // F-- I--P  0x49
  HID_KEY_TAB,                      // F-- I-R-  0x4A
  HID_KEY_HOME,                     // F-- I-RP  0x4B
  HID_KEY_UP_ARROW,                 // F-- IM--  0x4C
  HID_KEY_SCROLL_LOCK,              // F-- IM-P  0x4D
  HID_KEY_PAGE_UP,                  // F-- IMR-  0x4E
  HID_KEY_CAPS_LOCK,                // F-- IMRP  0x4F

  HID_KEY_PAUSE,                    // F-N ----  0x50
  HID_KEY_RESERVED,                 // F-N ---P  0x51
  HID_KEY_RESERVED,                 // F-N --R-  0x52
  HID_KEY_RESERVED,                 // F-N --RP  0x53
  HID_KEY_RESERVED,                 // F-N -M--  0x54
  HID_KEY_RESERVED,                 // F-N -M-P  0x55
  HID_KEY_RESERVED,                 // F-N -MR-  0x56
  HID_KEY_RESERVED,                 // F-N -MRP  0x57

  HID_KEY_RESERVED,                 // F-N I---  0x58
  HID_KEY_RESERVED,                 // F-N I--P  0x59
  HID_KEY_RESERVED,                 // F-N I-R-  0x5A
  HID_KEY_RESERVED,                 // F-N I-RP  0x5B
  HID_KEY_RESERVED,                 // F-N IM--  0x5C
  HID_KEY_RESERVED,                 // F-N IM-P  0x5D
  HID_KEY_RESERVED,                 // F-N IMR-  0x5E
  HID_KEY_RESERVED,                 // F-N IMRP  0x5F

  MOD_RSHIFT,                       // FC- ----  0x60
  HID_KEY_ENTER,                    // FC- ---P  0x61
  HID_KEYPAD_6,                     // FC- --R-  0x62
  MEDIA_volup,                      // FC- --RP  0x63
  MEDIA_stop,                       // FC- -M--  0x64
  HID_KEY_MULTIPLY,                 // FC- -M-P  0x65
  MEDIA_playpause,                  // FC- -MR-  0x66
  MEDIA_next,                       // FC- -MRP  0x67

  HID_KEYPAD_4,                     // FC- I---  0x68
  MEDIA_previous,                   // FC- I--P  0x69
  HID_KEY_SUBTRACT,                 // FC- I-R-  0x6A
  HID_KEYPAD_7,                     // FC- I-RP  0x6B
  MEDIA_voldn,                      // FC- IM--  0x6C
  HID_KEY_DIVIDE,                   // FC- IM-P  0x6D
  MEDIA_previous,                   // FC- IMR-  0x6E
  HID_KEYPAD_0,                     // FC- IMRP  0x6F

  MODE_MRESET,                      // FCN ----  0x70
  HID_KEY_RESERVED,                 // FCN ---P  0x71
  HID_KEY_RESERVED,                 // FCN --R-  0x72
  HID_KEY_RESERVED,                 // FCN --RP  0x73
  HID_KEY_RESERVED,                 // FCN -M--  0x74
  HID_KEY_RESERVED,                 // FCN -M-P  0x75
  HID_KEY_RESERVED,                 // FCN -MR-  0x76
  HID_KEY_RESERVED,                 // FCN -MRP  0x77

  HID_KEY_RESERVED,                 // FCN I---  0x78
  HID_KEY_RESERVED,                 // FCN I--P  0x79
  HID_KEY_RESERVED,                 // FCN I-R-  0x7A
  HID_KEY_RESERVED,                 // FCN I-RP  0x7B
  HID_KEY_RESERVED,                 // FCN IM--  0x7C
  HID_KEY_RESERVED,                 // FCN IM-P  0x7D
  HID_KEY_RESERVED,                 // FCN IMR-  0x7E
  HID_KEY_RESERVED                  // FCN IMRP  0x7F
};

/**************************************
 * number/symbols mode                *
 **************************************/
const keymap_t keymap_numsym[128] = {
  HID_KEY_RESERVED,                 // --- ----  0x00
  HID_KEY_5,                        // --- ---P  0x01
  HID_KEY_4,                        // --- --R-  0x02
  MACRO_quotes,                     // --- --RP  0x03   "" and a back arrow
  HID_KEY_3,                        // --- -M--  0x04
  HID_KEY_RESERVED,                 // --- -M-P  0x05
  MACRO_00,                         // --- -MR-  0x06   00
  HID_KEY_MINUS,                    // --- -MRP  0x07

  HID_KEY_2,                        // --- I---  0x08
  HID_KEY_BACK_SLASH,               // --- I--P  0x09
  MACRO_dollar,                     // --- I-R-  0x0A   $
  HID_KEY_GRV_ACCENT,               // --- I-RP  0x0B
  HID_KEY_FWD_SLASH,                // --- IM--  0x0C
  HID_KEY_RESERVED,                 // --- IM-P  0x0D
  HID_KEY_EQUAL,                    // --- IMR-  0x0E
  MACRO_000,                        // --- IMRP  0x0F   000

  HID_KEY_SPACEBAR,                 // --N ----  0x10
  MODE_FUNC,                        // --N ---P  0x11
  HID_KEY_ESCAPE,                   // --N --R-  0x12
  HID_KEY_SEMI_COLON,               // --N --RP  0x13
  HID_KEY_COMMA,                    // --N -M--  0x14
  HID_KEY_RESERVED,                 // --N -M-P  0x15
  HID_KEY_DOT,                      // --N -MR-  0x16
  MOD_LALT,                         // --N -MRP  0x17

  HID_KEY_RESERVED,                 // --N I---  0x18
  HID_KEY_INSERT,                   // --N I--P  0x19
  MOD_LGUI,                         // --N I-R-  0x1A
  MOD_LCTRL,                        // --N I-RP  0x1B
  HID_KEY_RESERVED,                 // --N IM--  0x1C
  HID_KEY_RESERVED,                 // --N IM-P  0x1D
  HID_KEY_SGL_QUOTE,                // --N IMR-  0x1E
  MODE_RESET,                       // --N IMRP  0x1F

  HID_KEY_1,                        // -C- ----  0x20
  HID_KEY_9,                        // -C- ---P  0x21
  HID_KEY_8,                        // -C- --R-  0x22
  HID_KEY_RIGHT_BRKT,               // -C- --RP  0x23
  HID_KEY_7,                        // -C- -M--  0x24
  HID_KEY_RIGHT_BRKT,               // -C- -M-P  0x25
  MACRO_percent,                    // -C- -MR-  0x26   %
  HID_KEY_LEFT_BRKT,                // -C- -MRP  0x27

  HID_KEY_6,                        // -C- I---  0x28
  MACRO_ampersand,                  // -C- I--P  0x29   &
  MACRO_parens,                     // -C- I-R-  0x2A   () and a back arrow
  MACRO_question,                   // -C- I-RP  0x2B   ?
  MACRO_asterisk,                   // -C- IM--  0x2C
  HID_KEY_LEFT_BRKT,                // -C- IM-P  0x2D
  MACRO_plus,                       // -C- IMR-  0x2E
  HID_KEY_0,                        // -C- IMRP  0x2F

  MULTI_NumShift,                   // -CN ----  0x30
  HID_KEY_RESERVED,                 // -CN ---P  0x31
  HID_KEY_RESERVED,                 // -CN --R-  0x32
  HID_KEY_RESERVED,                 // -CN --RP  0x33
  ANDROID_dpadcenter,               // -CN -M--  0x34
  HID_KEY_RESERVED,                 // -CN -M-P  0x35
  ANDROID_home,                     // -CN -MR-  0x36
  MOD_RALT,                         // -CN -MRP  0x37

  HID_KEY_RESERVED,                 // -CN I---  0x38
  ANDROID_back,                     // -CN I--P  0x39
  MOD_RGUI,                         // -CN I-R-  0x3A
  MOD_RCTRL,                        // -CN I-RP  0x3B
  ANDROID_menu,                     // -CN IM--  0x3C
  HID_KEY_RESERVED,                 // -CN IM-P  0x3D
  ANDROID_search,                   // -CN IMR-  0x3E
  HID_KEY_NUM_LOCK,                 // -CN IMRP  0x3F

  MOD_LSHIFT,                       // F-- ----  0x40
  HID_KEY_ENTER,                    // F-- ---P  0x41
  HID_KEY_RIGHT_ARROW,              // F-- --R-  0x42
  HID_KEY_DOWN_ARROW,               // F-- --RP  0x43
  HID_KEY_DELETE,                   // F-- -M--  0x44
  HID_KEY_PRNT_SCREEN,              // F-- -M-P  0x45
  HID_KEY_DELETE_FWD,               // F-- -MR-  0x46
  HID_KEY_PAGE_DOWN,                // F-- -MRP  0x47

  HID_KEY_LEFT_ARROW,               // F-- I---  0x48
  HID_KEY_END,                      // F-- I--P  0x49
  HID_KEY_TAB,                      // F-- I-R-  0x4A
  HID_KEY_HOME,                     // F-- I-RP  0x4B
  HID_KEY_UP_ARROW,                 // F-- IM--  0x4C
  HID_KEY_SCROLL_LOCK,              // F-- IM-P  0x4D
  HID_KEY_PAGE_UP,                  // F-- IMR-  0x4E
  HID_KEY_CAPS_LOCK,                // F-- IMRP  0x4F

  HID_KEY_PAUSE,                    // F-N ----  0x50
  HID_KEY_RESERVED,                 // F-N ---P  0x51
  HID_KEY_RESERVED,                 // F-N --R-  0x52
  HID_KEY_RESERVED,                 // F-N --RP  0x53
  HID_KEY_RESERVED,                 // F-N -M--  0x54
  HID_KEY_RESERVED,                 // F-N -M-P  0x55
  HID_KEY_RESERVED,                 // F-N -MR-  0x56
  HID_KEY_RESERVED,                 // F-N -MRP  0x57

  HID_KEY_RESERVED,                 // F-N I---  0x58
  HID_KEY_RESERVED,                 // F-N I--P  0x59
  HID_KEY_RESERVED,                 // F-N I-R-  0x5A
  HID_KEY_RESERVED,                 // F-N I-RP  0x5B
  HID_KEY_RESERVED,                 // F-N IM--  0x5C
  HID_KEY_RESERVED,                 // F-N IM-P  0x5D
  HID_KEY_RESERVED,                 // F-N IMR-  0x5E
  HID_KEY_RESERVED,                 // F-N IMRP  0x5F

  MOD_RSHIFT,                       // FC- ----  0x60
  HID_KEY_ENTER,                    // FC- ---P  0x61
  HID_KEYPAD_6,                     // FC- --R-  0x62
  HID_KEYPAD_2,                     // FC- --RP  0x63
  HID_KEYPAD_5,                     // FC- -M--  0x64
  HID_KEY_MULTIPLY,                 // FC- -M-P  0x65
  HID_KEYPAD_DOT,                   // FC- -MR-  0x66
  HID_KEYPAD_3,                     // FC- -MRP  0x67

  HID_KEYPAD_4,                     // FC- I---  0x68
  HID_KEYPAD_1,                     // FC- I--P  0x69
  HID_KEY_SUBTRACT,                 // FC- I-R-  0x6A
  HID_KEYPAD_7,                     // FC- I-RP  0x6B
  HID_KEYPAD_8,                     // FC- IM--  0x6C
  HID_KEY_DIVIDE,                   // FC- IM-P  0x6D
  HID_KEYPAD_9,                     // FC- IMR-  0x6E
  HID_KEYPAD_0,                     // FC- IMRP  0x6F

  MODE_RESET,                       // FCN ----  0x70
  HID_KEY_RESERVED,                 // FCN ---P  0x71
  HID_KEY_RESERVED,                 // FCN --R-  0x72
  HID_KEY_RESERVED,                 // FCN --RP  0x73
  HID_KEY_RESERVED,                 // FCN -M--  0x74
  HID_KEY_RESERVED,                 // FCN -M-P  0x75
  HID_KEY_RESERVED,                 // FCN -MR-  0x76
  HID_KEY_RESERVED,                 // FCN -MRP  0x77

  HID_KEY_RESERVED,                 // FCN I---  0x78
  HID_KEY_RESERVED,                 // FCN I--P  0x79
  HID_KEY_RESERVED,                 // FCN I-R-  0x7A
  HID_KEY_RESERVED,                 // FCN I-RP  0x7B
  HID_KEY_RESERVED,                 // FCN IM--  0x7C
  HID_KEY_RESERVED,                 // FCN IM-P  0x7D
  HID_KEY_RESERVED,                 // FCN IMR-  0x7E
  HID_KEY_RESERVED                  // FCN IMRP  0x7F
};

/**************************************
 * function key mode                  *
 **************************************/
const keymap_t keymap_function[128] = {
  HID_KEY_RESERVED,                 // --- ----  0x00
  HID_KEY_F5,                       // --- ---P  0x01
  HID_KEY_F4,                       // --- --R-  0x02
  MEDIA_volup,                      // --- --RP  0x03
  HID_KEY_F3,                       // --- -M--  0x04
  HID_KEY_RESERVED,                 // --- -M-P  0x05
  HID_KEY_RESERVED,                 // --- -MR-  0x06
  MEDIA_stop,                       // --- -MRP  0x07

  HID_KEY_F2,                       // --- I---  0x08
  MEDIA_previous,                   // --- I--P  0x09
  HID_KEY_RESERVED,                 // --- I-R-  0x0A
  HID_KEY_RESERVED,                 // --- I-RP  0x0B
  MEDIA_voldn,                      // --- IM--  0x0C
  HID_KEY_RESERVED,                 // --- IM-P  0x0D
  HID_KEY_RESERVED,                 // --- IMR-  0x0E
  HID_KEY_RESERVED,                 // --- IMRP  0x0F

  HID_KEY_RESERVED,                 // --N ----  0x10
  MODE_RESET,                       // --N ---P  0x11
  HID_KEY_RESERVED,                 // --N --R-  0x12
  HID_KEY_RESERVED,                 // --N --RP  0x13
  HID_KEY_RESERVED,                 // --N -M--  0x14
  HID_KEY_RESERVED,                 // --N -M-P  0x15
  HID_KEY_RESERVED,                 // --N -MR-  0x16
  MOD_LALT,                         // --N -MRP  0x17

  HID_KEY_RESERVED,                 // --N I---  0x18
  HID_KEY_RESERVED,                 // --N I--P  0x19
  MOD_LGUI,                         // --N I-R-  0x1A
  MOD_LCTRL,                        // --N I-RP  0x1B
  HID_KEY_RESERVED,                 // --N IM--  0x1C
  HID_KEY_RESERVED,                 // --N IM-P  0x1D
  HID_KEY_RESERVED,                 // --N IMR-  0x1E
  HID_KEY_RESERVED,                 // --N IMRP  0x1F

  HID_KEY_F1,                       // -C- ----  0x20
  HID_KEY_F9,                       // -C- ---P  0x21
  HID_KEY_F8,                       // -C- --R-  0x22
  HID_KEY_F12,                      // -C- --RP  0x23
  HID_KEY_F7,                       // -C- -M--  0x24
  HID_KEY_RESERVED,                 // -C- -M-P  0x25
  HID_KEY_F11,                      // -C- -MR-  0x26
  HID_KEY_RESERVED,                 // -C- -MRP  0x27

  HID_KEY_F6,                       // -C- I---  0x28
  HID_KEY_RESERVED,                 // -C- I--P  0x29
  HID_KEY_RESERVED,                 // -C- I-R-  0x2A
  HID_KEY_RESERVED,                 // -C- I-RP  0x2B
  HID_KEY_F10,                      // -C- IM--  0x2C
  HID_KEY_RESERVED,                 // -C- IM-P  0x2D
  HID_KEY_RESERVED,                 // -C- IMR-  0x2E
  HID_KEY_RESERVED,                 // -C- IMRP  0x2F

  MULTI_NumShift,                   // -CN ----  0x30
  HID_KEY_RESERVED,                 // -CN ---P  0x31
  HID_KEY_RESERVED,                 // -CN --R-  0x32
  HID_KEY_RESERVED,                 // -CN --RP  0x33
  ANDROID_dpadcenter,               // -CN -M--  0x34
  HID_KEY_RESERVED,                 // -CN -M-P  0x35
  ANDROID_home,                     // -CN -MR-  0x36
  MOD_RALT,                         // -CN -MRP  0x37

  HID_KEY_RESERVED,                 // -CN I---  0x38
  ANDROID_back,                     // -CN I--P  0x39
  MOD_RGUI,                         // -CN I-R-  0x3A
  MOD_RCTRL,                        // -CN I-RP  0x3B
  ANDROID_menu,                     // -CN IM--  0x3C
  HID_KEY_RESERVED,                 // -CN IM-P  0x3D
  ANDROID_search,                   // -CN IMR-  0x3E
  HID_KEY_RESERVED,                 // -CN IMRP  0x3F

  MOD_LSHIFT,                       // F-- ----  0x40
  HID_KEY_RESERVED,                 // F-- ---P  0x41
  HID_KEY_RESERVED,                 // F-- --R-  0x42
  HID_KEY_RESERVED,                 // F-- --RP  0x43
  HID_KEY_RESERVED,                 // F-- -M--  0x44
  HID_KEY_RESERVED,                 // F-- -M-P  0x45
  MEDIA_playpause,                  // F-- -MR-  0x46
  MEDIA_next,                       // F-- -MRP  0x47

  MEDIA_previous,                   // F-- I---  0x48
  HID_KEY_RESERVED,                 // F-- I--P  0x49
  HID_KEY_RESERVED,                 // F-- I-R-  0x4A
  HID_KEY_RESERVED,                 // F-- I-RP  0x4B
  HID_KEY_RESERVED,                 // F-- IM--  0x4C
  HID_KEY_RESERVED,                 // F-- IM-P  0x4D
  HID_KEY_RESERVED,                 // F-- IMR-  0x4E
  HID_KEY_RESERVED,                 // F-- IMRP  0x4F

  HID_KEY_RESERVED,                 // F-N ----  0x50
  HID_KEY_RESERVED,                 // F-N ---P  0x51
  HID_KEY_RESERVED,                 // F-N --R-  0x52
  HID_KEY_RESERVED,                 // F-N --RP  0x53
  HID_KEY_RESERVED,                 // F-N -M--  0x54
  HID_KEY_RESERVED,                 // F-N -M-P  0x55
  HID_KEY_RESERVED,                 // F-N -MR-  0x56
  HID_KEY_RESERVED,                 // F-N -MRP  0x57

  HID_KEY_RESERVED,                 // F-N I---  0x58
  HID_KEY_RESERVED,                 // F-N I--P  0x59
  HID_KEY_RESERVED,                 // F-N I-R-  0x5A
  HID_KEY_RESERVED,                 // F-N I-RP  0x5B
  HID_KEY_RESERVED,                 // F-N IM--  0x5C
  HID_KEY_RESERVED,                 // F-N IM-P  0x5D
  HID_KEY_RESERVED,                 // F-N IMR-  0x5E
  HID_KEY_RESERVED,                 // F-N IMRP  0x5F

  MOD_RSHIFT,                       // FC- ----  0x60
  HID_KEY_RESERVED,                 // FC- ---P  0x61
  HID_KEY_RESERVED,                 // FC- --R-  0x62
  HID_KEY_RESERVED,                 // FC- --RP  0x63
  HID_KEY_RESERVED,                 // FC- -M--  0x64
  HID_KEY_RESERVED,                 // FC- -M-P  0x65
  HID_KEY_RESERVED,                 // FC- -MR-  0x66
  HID_KEY_RESERVED,                 // FC- -MRP  0x67

  HID_KEY_RESERVED,                 // FC- I---  0x68
  HID_KEY_RESERVED,                 // FC- I--P  0x69
  HID_KEY_RESERVED,                 // FC- I-R-  0x6A
  HID_KEY_RESERVED,                 // FC- I-RP  0x6B
  HID_KEY_RESERVED,                 // FC- IM--  0x6C
  HID_KEY_RESERVED,                 // FC- IM-P  0x6D
  HID_KEY_RESERVED,                 // FC- IMR-  0x6E
  HID_KEY_RESERVED,                 // FC- IMRP  0x6F

  MODE_RESET,                       // FCN ----  0x70
  HID_KEY_RESERVED,                 // FCN ---P  0x71
  HID_KEY_RESERVED,                 // FCN --R-  0x72
  HID_KEY_RESERVED,                 // FCN --RP  0x73
  HID_KEY_RESERVED,                 // FCN -M--  0x74
  HID_KEY_RESERVED,                 // FCN -M-P  0x75
  HID_KEY_RESERVED,                 // FCN -MR-  0x76
  HID_KEY_RESERVED,                 // FCN -MRP  0x77

  HID_KEY_RESERVED,                 // FCN I---  0x78
  HID_KEY_RESERVED,                 // FCN I--P  0x79
  HID_KEY_RESERVED,                 // FCN I-R-  0x7A
  HID_KEY_RESERVED,                 // FCN I-RP  0x7B
  HID_KEY_RESERVED,                 // FCN IM--  0x7C
  HID_KEY_RESERVED,                 // FCN IM-P  0x7D
  HID_KEY_RESERVED,                 // FCN IMR-  0x7E
  HID_KEY_RESERVED                  // FCN IMRP  0x7F
};

// end ChordMappings.h
