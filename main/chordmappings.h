// ChordMappings.h
// Chordmappings for 7 button chorder, split out from FeatherChorder.ino
// Mappings moved here so they can be changed without risk
// of modifying the rest of the code.
// - Greg

#include "hid_dev.h"
#include <stdint.h>

typedef uint16_t keymap_t;

typedef uint16_t symbol_t;

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
  MODE_NOTETAKING,      // Switch to non-BLE note taking
  MODE_BLE_KEYBOARD,    // Switch to BLE keyboard mode
  MODE_BLE_MOUSE,       // Switch to BLE mouse mode

/* Further keys for non-BLE behaviour */
  DIV_NonBLE,
  NONBLE_NOKEY=DIV_NonBLE, // No matching key, do nothing
  NONBLE_LEFTARR,
  NONBLE_DOWNARR,
  NONBLE_UPARR,
  NONBLE_RIGHTARR,
  NONBLE_BACKSPACE,

/* Then a special mode for both num/sym and shift */
  DIV_Multi,
  MULTI_NumShift=DIV_Multi,
  MULTI_CtlAlt,

/* Bluetooth mouse controls */
  BLEMOUSE_LEFT,
  BLEMOUSE_DOWN,
  BLEMOUSE_UP,
  BLEMOUSE_RIGHT,
  BLEMOUSE_1CLICK,
  BLEMOUSE_2CLICK,
  BLEMOUSE_3CLICK,
  BLEMOUSE_1TOGGLE,
  BLEMOUSE_2TOGGLE,
  BLEMOUSE_3TOGGLE,
  BLEMOUSE_FURTHER,
  BLEMOUSE_SHORTER,

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
 *  You have three thumb buttons: Far, Center and Near    *
 *  Further, you have: Index, Middle, Ring, Pinky         *
 *  Denoted (right hand) like this: FCN IMRP              *
 **********************************************************/

/**
 * Order here is:
 * - mode == ALPHA,
 * - mode == NUMSYM,
 * - mode == FUNCTION,
 * - char when unshifted
 * - char when shifted
 * - char when number-prefixed
 * - 
 */

symbol_t keymap[128][7] = { // {{{
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for --- ----  0x00 no keys pressed 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --- ----  0x00 no keys pressed 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- ----  0x00 no keys pressed 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- ----  0x00 no keys pressed
  NONBLE_NOKEY             , // non-BLE shifted   for --- ----  0x00 no keys pressed 
  NONBLE_NOKEY             , // non-BLE unshifted for --- ----  0x00 no keys pressed 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- ----  0x00 no keys pressed 
},
{
  HID_KEY_W                , // MODE==ALPHA       for --- ---P  0x01 
  HID_KEY_5                , // MODE==NUMSYM      for --- ---P  0x01 
  HID_KEY_F5               , // MODE==FUNCTION    for --- ---P  0x01 
  BLEMOUSE_RIGHT           , // MODE==MOUSE       for --- ---P  0x01
  'W'                      , // non-BLE shifted   for --- ---P  0x01 
  'w'                      , // non-BLE unshifted for --- ---P  0x01 
  '5'                      , // non-BLE numsymed  for --- ---P  0x01 
},
{
  HID_KEY_Y                , // MODE==ALPHA       for --- --R-  0x02 
  HID_KEY_4                , // MODE==NUMSYM      for --- --R-  0x02 
  HID_KEY_F4               , // MODE==FUNCTION    for --- --R-  0x02 
  BLEMOUSE_UP              , // MODE==MOUSE       for --- --R-  0x02
  'Y'                      , // non-BLE shifted   for --- --R-  0x02 
  'y'                      , // non-BLE unshifted for --- --R-  0x02 
  '4'                      , // non-BLE numsymed  for --- --R-  0x02 
},
{
  HID_KEY_U                , // MODE==ALPHA       for --- --RP  0x03 
  MACRO_quotes             , // MODE==NUMSYM      for --- --RP  0x03 
  MEDIA_volup              , // MODE==FUNCTION    for --- --RP  0x03 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- --RP  0x03
  'U'                      , // non-BLE shifted   for --- --RP  0x03 
  'u'                      , // non-BLE unshifted for --- --RP  0x03 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- --RP  0x03 
},
{
  HID_KEY_R                , // MODE==ALPHA       for --- -M--  0x04 
  HID_KEY_3                , // MODE==NUMSYM      for --- -M--  0x04 
  HID_KEY_F3               , // MODE==FUNCTION    for --- -M--  0x04 
  BLEMOUSE_DOWN            , // MODE==MOUSE       for --- -M--  0x04
  'R'                      , // non-BLE shifted   for --- -M--  0x04 
  'r'                      , // non-BLE unshifted for --- -M--  0x04 
  '3'                      , // non-BLE numsymed  for --- -M--  0x04 
},
{
  MACRO_closeparen         , // MODE==ALPHA       for --- -M-P  0x05 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --- -M-P  0x05 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- -M-P  0x05 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- -M-P  0x05
  ')'                      , // non-BLE shifted   for --- -M-P  0x05 
  ')'                      , // non-BLE unshifted for --- -M-P  0x05 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- -M-P  0x05 
},
{
  HID_KEY_H                , // MODE==ALPHA       for --- -MR-  0x06 
  MACRO_00                 , // MODE==NUMSYM      for --- -MR-  0x06 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- -MR-  0x06 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- -MR-  0x06
  'H'                      , // non-BLE shifted   for --- -MR-  0x06 
  'h'                      , // non-BLE unshifted for --- -MR-  0x06 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- -MR-  0x06 
},
{
  HID_KEY_S                , // MODE==ALPHA       for --- -MRP  0x07 
  HID_KEY_MINUS            , // MODE==NUMSYM      for --- -MRP  0x07 
  MEDIA_stop               , // MODE==FUNCTION    for --- -MRP  0x07 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- -MRP  0x07
  'S'                      , // non-BLE shifted   for --- -MRP  0x07 
  's'                      , // non-BLE unshifted for --- -MRP  0x07 
  '-'                      , // non-BLE numsymed  for --- -MRP  0x07 
},
{
  HID_KEY_I                , // MODE==ALPHA       for --- I---  0x08 
  HID_KEY_2                , // MODE==NUMSYM      for --- I---  0x08 
  HID_KEY_F2               , // MODE==FUNCTION    for --- I---  0x08 
  BLEMOUSE_LEFT            , // MODE==MOUSE       for --- I---  0x08
  'I'                      , // non-BLE shifted   for --- I---  0x08 
  'i'                      , // non-BLE unshifted for --- I---  0x08 
  '2'                      , // non-BLE numsymed  for --- I---  0x08 
},
{
  HID_KEY_B                , // MODE==ALPHA       for --- I--P  0x09 
  HID_KEY_BACK_SLASH       , // MODE==NUMSYM      for --- I--P  0x09 
  MEDIA_previous           , // MODE==FUNCTION    for --- I--P  0x09 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- I--P  0x09
  'B'                      , // non-BLE shifted   for --- I--P  0x09 
  'b'                      , // non-BLE unshifted for --- I--P  0x09 
  '\\'                     , // non-BLE numsymed  for --- I--P  0x09 
},
{
  HID_KEY_K                , // MODE==ALPHA       for --- I-R-  0x0A 
  MACRO_dollar             , // MODE==NUMSYM      for --- I-R-  0x0A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- I-R-  0x0A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- I-R-  0x0A
  'K'                      , // non-BLE shifted   for --- I-R-  0x0A 
  'k'                      , // non-BLE unshifted for --- I-R-  0x0A 
  '$'                      , // non-BLE numsymed  for --- I-R-  0x0A 
},
{
  HID_KEY_Z                , // MODE==ALPHA       for --- I-RP  0x0B 
  HID_KEY_GRV_ACCENT       , // MODE==NUMSYM      for --- I-RP  0x0B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- I-RP  0x0B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- I-RP  0x0B
  'Z'                      , // non-BLE shifted   for --- I-RP  0x0B 
  'z'                      , // non-BLE unshifted for --- I-RP  0x0B 
  '`'                      , // non-BLE numsymed  for --- I-RP  0x0B 
},
{
  HID_KEY_D                , // MODE==ALPHA       for --- IM--  0x0C 
  HID_KEY_FWD_SLASH        , // MODE==NUMSYM      for --- IM--  0x0C 
  MEDIA_voldn              , // MODE==FUNCTION    for --- IM--  0x0C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- IM--  0x0C
  'D'                      , // non-BLE shifted   for --- IM--  0x0C 
  'd'                      , // non-BLE unshifted for --- IM--  0x0C 
  '/'                      , // non-BLE numsymed  for --- IM--  0x0C 
},
{
  MACRO_openparen          , // MODE==ALPHA       for --- IM-P  0x0D 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --- IM-P  0x0D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- IM-P  0x0D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- IM-P  0x0D
  '('                      , // non-BLE shifted   for --- IM-P  0x0D 
  '('                      , // non-BLE unshifted for --- IM-P  0x0D 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- IM-P  0x0D 
},
{
  HID_KEY_E                , // MODE==ALPHA       for --- IMR-  0x0E 
  HID_KEY_EQUAL            , // MODE==NUMSYM      for --- IMR-  0x0E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- IMR-  0x0E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- IMR-  0x0E
  'E'                      , // non-BLE shifted   for --- IMR-  0x0E 
  'e'                      , // non-BLE unshifted for --- IMR-  0x0E 
  '='                      , // non-BLE numsymed  for --- IMR-  0x0E 
},
{
  HID_KEY_T                , // MODE==ALPHA       for --- IMRP  0x0F 
  MACRO_000                , // MODE==NUMSYM      for --- IMRP  0x0F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --- IMRP  0x0F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --- IMRP  0x0F
  'T'                      , // non-BLE shifted   for --- IMRP  0x0F 
  't'                      , // non-BLE unshifted for --- IMRP  0x0F 
  NONBLE_NOKEY             , // non-BLE numsymed  for --- IMRP  0x0F 
},
{
  MODE_NUM                 , // MODE==ALPHA       for --N ----  0x10 
  HID_KEY_SPACEBAR         , // MODE==NUMSYM      for --N ----  0x10 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N ----  0x10 
  BLEMOUSE_FURTHER         , // MODE==MOUSE       for --N ----  0x10
  MODE_NUM                 , // non-BLE shifted   for --N ----  0x10 
  MODE_NUM                 , // non-BLE unshifted for --N ----  0x10 
  ' '                      , // non-BLE numsymed  for --N ----  0x10 
},
{
  MODE_FUNC                , // MODE==ALPHA       for --N ---P  0x11 
  MODE_FUNC                , // MODE==NUMSYM      for --N ---P  0x11 
  MODE_RESET               , // MODE==FUNCTION    for --N ---P  0x11 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N ---P  0x11
  MODE_FUNC                , // non-BLE shifted   for --N ---P  0x11 
  MODE_FUNC                , // non-BLE unshifted for --N ---P  0x11 
  MODE_FUNC                , // non-BLE numsymed  for --N ---P  0x11 
},
{
  HID_KEY_ESCAPE           , // MODE==ALPHA       for --N --R-  0x12 
  HID_KEY_ESCAPE           , // MODE==NUMSYM      for --N --R-  0x12 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N --R-  0x12 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N --R-  0x12
  NONBLE_NOKEY             , // non-BLE shifted   for --N --R-  0x12 
  NONBLE_NOKEY             , // non-BLE unshifted for --N --R-  0x12 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N --R-  0x12 
},
{
  HID_KEY_SEMI_COLON       , // MODE==ALPHA       for --N --RP  0x13 
  HID_KEY_SEMI_COLON       , // MODE==NUMSYM      for --N --RP  0x13 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N --RP  0x13 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N --RP  0x13
  ';'                      , // non-BLE shifted   for --N --RP  0x13 
  ';'                      , // non-BLE unshifted for --N --RP  0x13 
  ';'                      , // non-BLE numsymed  for --N --RP  0x13 
},
{
  HID_KEY_COMMA            , // MODE==ALPHA       for --N -M--  0x14 
  HID_KEY_COMMA            , // MODE==NUMSYM      for --N -M--  0x14 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N -M--  0x14 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N -M--  0x14
  '<'                      , // non-BLE shifted   for --N -M--  0x14 
  ','                      , // non-BLE unshifted for --N -M--  0x14 
  ','                      , // non-BLE numsymed  for --N -M--  0x14 
},
{
  MACRO_closecurly         , // MODE==ALPHA       for --N -M-P  0x15 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --N -M-P  0x15 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N -M-P  0x15 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N -M-P  0x15
  '}'                      , // non-BLE shifted   for --N -M-P  0x15 
  '}'                      , // non-BLE unshifted for --N -M-P  0x15 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N -M-P  0x15 
},
{
  HID_KEY_DOT              , // MODE==ALPHA       for --N -MR-  0x16 
  HID_KEY_DOT              , // MODE==NUMSYM      for --N -MR-  0x16 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N -MR-  0x16 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N -MR-  0x16
  '>'                      , // non-BLE shifted   for --N -MR-  0x16 
  '.'                      , // non-BLE unshifted for --N -MR-  0x16 
  '.'                      , // non-BLE numsymed  for --N -MR-  0x16 
},
{
  MOD_LALT                 , // MODE==ALPHA       for --N -MRP  0x17 
  MOD_LALT                 , // MODE==NUMSYM      for --N -MRP  0x17 
  MOD_LALT                 , // MODE==FUNCTION    for --N -MRP  0x17 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N -MRP  0x17
  MOD_LALT                 , // non-BLE shifted   for --N -MRP  0x17 
  MOD_LALT                 , // non-BLE unshifted for --N -MRP  0x17 
  MOD_LALT                 , // non-BLE numsymed  for --N -MRP  0x17 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for --N I---  0x18 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --N I---  0x18 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N I---  0x18 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N I---  0x18
  NONBLE_NOKEY             , // non-BLE shifted   for --N I---  0x18 
  NONBLE_NOKEY             , // non-BLE unshifted for --N I---  0x18 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N I---  0x18 
},
{
  HID_KEY_INSERT           , // MODE==ALPHA       for --N I--P  0x19 
  HID_KEY_INSERT           , // MODE==NUMSYM      for --N I--P  0x19 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N I--P  0x19 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N I--P  0x19
  NONBLE_NOKEY             , // non-BLE shifted   for --N I--P  0x19 
  NONBLE_NOKEY             , // non-BLE unshifted for --N I--P  0x19 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N I--P  0x19 
},
{
  MOD_LGUI                 , // MODE==ALPHA       for --N I-R-  0x1A 
  MOD_LGUI                 , // MODE==NUMSYM      for --N I-R-  0x1A 
  MOD_LGUI                 , // MODE==FUNCTION    for --N I-R-  0x1A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N I-R-  0x1A
  NONBLE_NOKEY             , // non-BLE shifted   for --N I-R-  0x1A 
  NONBLE_NOKEY             , // non-BLE unshifted for --N I-R-  0x1A 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N I-R-  0x1A 
},
{
  MOD_LCTRL                , // MODE==ALPHA       for --N I-RP  0x1B 
  MOD_LCTRL                , // MODE==NUMSYM      for --N I-RP  0x1B 
  MOD_LCTRL                , // MODE==FUNCTION    for --N I-RP  0x1B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N I-RP  0x1B
  NONBLE_NOKEY             , // non-BLE shifted   for --N I-RP  0x1B 
  NONBLE_NOKEY             , // non-BLE unshifted for --N I-RP  0x1B 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N I-RP  0x1B 
},
{
  HID_KEY_F9               , // MODE==ALPHA       for --N IM--  0x1C 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --N IM--  0x1C 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N IM--  0x1C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N IM--  0x1C
  NONBLE_NOKEY             , // non-BLE shifted   for --N IM--  0x1C 
  NONBLE_NOKEY             , // non-BLE unshifted for --N IM--  0x1C 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N IM--  0x1C 
},
{
  MACRO_opencurly          , // MODE==ALPHA       for --N IM-P  0x1D 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for --N IM-P  0x1D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N IM-P  0x1D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N IM-P  0x1D
  '{'                      , // non-BLE shifted   for --N IM-P  0x1D 
  '{'                      , // non-BLE unshifted for --N IM-P  0x1D 
  NONBLE_NOKEY             , // non-BLE numsymed  for --N IM-P  0x1D 
},
{
  HID_KEY_SGL_QUOTE        , // MODE==ALPHA       for --N IMR-  0x1E 
  HID_KEY_SGL_QUOTE        , // MODE==NUMSYM      for --N IMR-  0x1E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N IMR-  0x1E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N IMR-  0x1E
  '\''                     , // non-BLE shifted   for --N IMR-  0x1E 
  '\''                     , // non-BLE unshifted for --N IMR-  0x1E 
  '\''                     , // non-BLE numsymed  for --N IMR-  0x1E 
},
{
  HID_KEY_NUM_LOCK         , // MODE==ALPHA       for --N IMRP  0x1F 
  MODE_RESET               , // MODE==NUMSYM      for --N IMRP  0x1F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for --N IMRP  0x1F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for --N IMRP  0x1F
  NONBLE_NOKEY             , // non-BLE shifted   for --N IMRP  0x1F 
  NONBLE_NOKEY             , // non-BLE unshifted for --N IMRP  0x1F 
  MODE_RESET               , // non-BLE numsymed  for --N IMRP  0x1F 
},
{
  HID_KEY_SPACEBAR         , // MODE==ALPHA       for -C- ----  0x20 
  HID_KEY_1                , // MODE==NUMSYM      for -C- ----  0x20 
  HID_KEY_F1               , // MODE==FUNCTION    for -C- ----  0x20 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- ----  0x20
  ' '                      , // non-BLE shifted   for -C- ----  0x20 
  ' '                      , // non-BLE unshifted for -C- ----  0x20 
  '1'                      , // non-BLE numsymed  for -C- ----  0x20 
},
{
  HID_KEY_F                , // MODE==ALPHA       for -C- ---P  0x21 
  HID_KEY_9                , // MODE==NUMSYM      for -C- ---P  0x21 
  HID_KEY_F9               , // MODE==FUNCTION    for -C- ---P  0x21 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- ---P  0x21
  'F'                      , // non-BLE shifted   for -C- ---P  0x21 
  'f'                      , // non-BLE unshifted for -C- ---P  0x21 
  '9'                      , // non-BLE numsymed  for -C- ---P  0x21 
},
{
  HID_KEY_G                , // MODE==ALPHA       for -C- --R-  0x22 
  HID_KEY_8                , // MODE==NUMSYM      for -C- --R-  0x22 
  HID_KEY_F8               , // MODE==FUNCTION    for -C- --R-  0x22 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- --R-  0x22
  'G'                      , // non-BLE shifted   for -C- --R-  0x22 
  'g'                      , // non-BLE unshifted for -C- --R-  0x22 
  '8'                      , // non-BLE numsymed  for -C- --R-  0x22 
},
{
  HID_KEY_V                , // MODE==ALPHA       for -C- --RP  0x23 
  HID_KEY_RIGHT_BRKT       , // MODE==NUMSYM      for -C- --RP  0x23 
  HID_KEY_F12              , // MODE==FUNCTION    for -C- --RP  0x23 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- --RP  0x23
  'V'                      , // non-BLE shifted   for -C- --RP  0x23 
  'v'                      , // non-BLE unshifted for -C- --RP  0x23 
  ']'                      , // non-BLE numsymed  for -C- --RP  0x23 
},
{
  HID_KEY_C                , // MODE==ALPHA       for -C- -M--  0x24 
  HID_KEY_7                , // MODE==NUMSYM      for -C- -M--  0x24 
  HID_KEY_F7               , // MODE==FUNCTION    for -C- -M--  0x24 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- -M--  0x24
  'C'                      , // non-BLE shifted   for -C- -M--  0x24 
  'c'                      , // non-BLE unshifted for -C- -M--  0x24 
  '7'                      , // non-BLE numsymed  for -C- -M--  0x24 
},
{
  HID_KEY_RIGHT_BRKT       , // MODE==ALPHA       for -C- -M-P  0x25 
  HID_KEY_RIGHT_BRKT       , // MODE==NUMSYM      for -C- -M-P  0x25 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- -M-P  0x25 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- -M-P  0x25
  '['                      , // non-BLE shifted   for -C- -M-P  0x25 
  '['                      , // non-BLE unshifted for -C- -M-P  0x25 
  ']'                      , // non-BLE numsymed  for -C- -M-P  0x25 
},
{
  HID_KEY_P                , // MODE==ALPHA       for -C- -MR-  0x26 
  MACRO_percent            , // MODE==NUMSYM      for -C- -MR-  0x26 
  HID_KEY_F11              , // MODE==FUNCTION    for -C- -MR-  0x26 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- -MR-  0x26
  'P'                      , // non-BLE shifted   for -C- -MR-  0x26 
  'p'                      , // non-BLE unshifted for -C- -MR-  0x26 
  '%'                      , // non-BLE numsymed  for -C- -MR-  0x26 
},
{
  HID_KEY_N                , // MODE==ALPHA       for -C- -MRP  0x27 
  HID_KEY_LEFT_BRKT        , // MODE==NUMSYM      for -C- -MRP  0x27 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- -MRP  0x27 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- -MRP  0x27
  'N'                      , // non-BLE shifted   for -C- -MRP  0x27 
  'n'                      , // non-BLE unshifted for -C- -MRP  0x27 
  '['                      , // non-BLE numsymed  for -C- -MRP  0x27 
},
{
  HID_KEY_L                , // MODE==ALPHA       for -C- I---  0x28 
  HID_KEY_6                , // MODE==NUMSYM      for -C- I---  0x28 
  HID_KEY_F6               , // MODE==FUNCTION    for -C- I---  0x28 
  BLEMOUSE_1CLICK,           // MODE==MOUSE       for -C- I---  0x28
  'L'                      , // non-BLE shifted   for -C- I---  0x28 
  'l'                      , // non-BLE unshifted for -C- I---  0x28 
  '6'                      , // non-BLE numsymed  for -C- I---  0x28 
},
{
  HID_KEY_X                , // MODE==ALPHA       for -C- I--P  0x29 
  MACRO_ampersand          , // MODE==NUMSYM      for -C- I--P  0x29 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- I--P  0x29 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- I--P  0x29
  'X'                      , // non-BLE shifted   for -C- I--P  0x29 
  'x'                      , // non-BLE unshifted for -C- I--P  0x29 
  '&'                      , // non-BLE numsymed  for -C- I--P  0x29 
},
{
  HID_KEY_J                , // MODE==ALPHA       for -C- I-R-  0x2A 
  MACRO_parens             , // MODE==NUMSYM      for -C- I-R-  0x2A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- I-R-  0x2A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- I-R-  0x2A
  'J'                      , // non-BLE shifted   for -C- I-R-  0x2A 
  'j'                      , // non-BLE unshifted for -C- I-R-  0x2A 
  NONBLE_NOKEY             , // non-BLE numsymed  for -C- I-R-  0x2A 
},
{
  HID_KEY_Q                , // MODE==ALPHA       for -C- I-RP  0x2B 
  MACRO_question           , // MODE==NUMSYM      for -C- I-RP  0x2B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- I-RP  0x2B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- I-RP  0x2B
  'Q'                      , // non-BLE shifted   for -C- I-RP  0x2B 
  'q'                      , // non-BLE unshifted for -C- I-RP  0x2B 
  '?'                      , // non-BLE numsymed  for -C- I-RP  0x2B 
},
{
  HID_KEY_M                , // MODE==ALPHA       for -C- IM--  0x2C 
  MACRO_asterisk           , // MODE==NUMSYM      for -C- IM--  0x2C 
  HID_KEY_F10              , // MODE==FUNCTION    for -C- IM--  0x2C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- IM--  0x2C
  'M'                      , // non-BLE shifted   for -C- IM--  0x2C 
  'm'                      , // non-BLE unshifted for -C- IM--  0x2C 
  '*'                      , // non-BLE numsymed  for -C- IM--  0x2C 
},
{
  HID_KEY_LEFT_BRKT        , // MODE==ALPHA       for -C- IM-P  0x2D 
  HID_KEY_LEFT_BRKT        , // MODE==NUMSYM      for -C- IM-P  0x2D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- IM-P  0x2D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- IM-P  0x2D
  ']'                      , // non-BLE shifted   for -C- IM-P  0x2D 
  ']'                      , // non-BLE unshifted for -C- IM-P  0x2D 
  '['                      , // non-BLE numsymed  for -C- IM-P  0x2D 
},
{
  HID_KEY_A                , // MODE==ALPHA       for -C- IMR-  0x2E 
  MACRO_plus               , // MODE==NUMSYM      for -C- IMR-  0x2E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- IMR-  0x2E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- IMR-  0x2E
  'A'                      , // non-BLE shifted   for -C- IMR-  0x2E 
  'a'                      , // non-BLE unshifted for -C- IMR-  0x2E 
  '+'                      , // non-BLE numsymed  for -C- IMR-  0x2E 
},
{
  HID_KEY_O                , // MODE==ALPHA       for -C- IMRP  0x2F 
  HID_KEY_0                , // MODE==NUMSYM      for -C- IMRP  0x2F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -C- IMRP  0x2F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -C- IMRP  0x2F
  'O'                      , // non-BLE shifted   for -C- IMRP  0x2F 
  'o'                      , // non-BLE unshifted for -C- IMRP  0x2F 
  '0'                      , // non-BLE numsymed  for -C- IMRP  0x2F 
},
{
  MULTI_NumShift           , // MODE==ALPHA       for -CN ----  0x30 
  MULTI_NumShift           , // MODE==NUMSYM      for -CN ----  0x30 
  MULTI_NumShift           , // MODE==FUNCTION    for -CN ----  0x30 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN ----  0x30
  NONBLE_NOKEY             , // non-BLE shifted   for -CN ----  0x30 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN ----  0x30 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN ----  0x30 
},
{
  MODE_NOTETAKING          , // MODE==ALPHA       for -CN ---P  0x31 
  MODE_NOTETAKING          , // MODE==NUMSYM      for -CN ---P  0x31 
  MODE_NOTETAKING          , // MODE==FUNCTION    for -CN ---P  0x31 
  MODE_NOTETAKING          , // MODE==MOUSE       for -CN ---P  0x31
  MODE_NOTETAKING          , // non-BLE shifted   for -CN ---P  0x31 
  MODE_NOTETAKING          , // non-BLE unshifted for -CN ---P  0x31 
  MODE_NOTETAKING          , // non-BLE numsymed  for -CN ---P  0x31 
},
{
  MODE_BLE_KEYBOARD        , // MODE==ALPHA       for -CN --R-  0x32 
  MODE_BLE_KEYBOARD        , // MODE==NUMSYM      for -CN --R-  0x32 
  MODE_BLE_KEYBOARD        , // MODE==FUNCTION    for -CN --R-  0x32 
  MODE_BLE_KEYBOARD        , // MODE==MOUSE       for -CN --R-  0x32
  MODE_BLE_KEYBOARD        , // non-BLE shifted   for -CN --R-  0x32 
  MODE_BLE_KEYBOARD        , // non-BLE unshifted for -CN --R-  0x32 
  MODE_BLE_KEYBOARD        , // non-BLE numsymed  for -CN --R-  0x32 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for -CN --RP  0x33 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for -CN --RP  0x33 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -CN --RP  0x33 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN --RP  0x33
  NONBLE_NOKEY             , // non-BLE shifted   for -CN --RP  0x33 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN --RP  0x33 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN --RP  0x33 
},
{
  MODE_BLE_MOUSE           , // MODE==ALPHA       for -CN -M--  0x34 
  MODE_BLE_MOUSE           , // MODE==NUMSYM      for -CN -M--  0x34 
  MODE_BLE_MOUSE           , // MODE==FUNCTION    for -CN -M--  0x34 
  MODE_BLE_MOUSE           , // MODE==MOUSE       for -CN -M--  0x34
  MODE_BLE_MOUSE           , // non-BLE shifted   for -CN -M--  0x34 
  MODE_BLE_MOUSE           , // non-BLE unshifted for -CN -M--  0x34 
  MODE_BLE_MOUSE           , // non-BLE numsymed  for -CN -M--  0x34 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for -CN -M-P  0x35 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for -CN -M-P  0x35 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -CN -M-P  0x35 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN -M-P  0x35
  NONBLE_NOKEY             , // non-BLE shifted   for -CN -M-P  0x35 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN -M-P  0x35 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN -M-P  0x35 
},
{
  ANDROID_home             , // MODE==ALPHA       for -CN -MR-  0x36 
  ANDROID_home             , // MODE==NUMSYM      for -CN -MR-  0x36 
  ANDROID_home             , // MODE==FUNCTION    for -CN -MR-  0x36 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN -MR-  0x36
  NONBLE_NOKEY             , // non-BLE shifted   for -CN -MR-  0x36 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN -MR-  0x36 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN -MR-  0x36 
},
{
  MOD_RALT                 , // MODE==ALPHA       for -CN -MRP  0x37 
  MOD_RALT                 , // MODE==NUMSYM      for -CN -MRP  0x37 
  MOD_RALT                 , // MODE==FUNCTION    for -CN -MRP  0x37 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN -MRP  0x37
  MOD_RALT                 , // non-BLE shifted   for -CN -MRP  0x37 
  MOD_RALT                 , // non-BLE unshifted for -CN -MRP  0x37 
  MOD_RALT                 , // non-BLE numsymed  for -CN -MRP  0x37 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for -CN I---  0x38 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for -CN I---  0x38 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -CN I---  0x38 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN I---  0x38
  NONBLE_NOKEY             , // non-BLE shifted   for -CN I---  0x38 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN I---  0x38 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN I---  0x38 
},
{
  ANDROID_back             , // MODE==ALPHA       for -CN I--P  0x39 
  ANDROID_back             , // MODE==NUMSYM      for -CN I--P  0x39 
  ANDROID_back             , // MODE==FUNCTION    for -CN I--P  0x39 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN I--P  0x39
  NONBLE_NOKEY             , // non-BLE shifted   for -CN I--P  0x39 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN I--P  0x39 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN I--P  0x39 
},
{
  MOD_RGUI                 , // MODE==ALPHA       for -CN I-R-  0x3A 
  MOD_RGUI                 , // MODE==NUMSYM      for -CN I-R-  0x3A 
  MOD_RGUI                 , // MODE==FUNCTION    for -CN I-R-  0x3A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN I-R-  0x3A
  NONBLE_NOKEY             , // non-BLE shifted   for -CN I-R-  0x3A 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN I-R-  0x3A 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN I-R-  0x3A 
},
{
  MOD_RCTRL                , // MODE==ALPHA       for -CN I-RP  0x3B 
  MOD_RCTRL                , // MODE==NUMSYM      for -CN I-RP  0x3B 
  MOD_RCTRL                , // MODE==FUNCTION    for -CN I-RP  0x3B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN I-RP  0x3B
  NONBLE_NOKEY             , // non-BLE shifted   for -CN I-RP  0x3B 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN I-RP  0x3B 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN I-RP  0x3B 
},
{
  ANDROID_menu             , // MODE==ALPHA       for -CN IM--  0x3C 
  ANDROID_menu             , // MODE==NUMSYM      for -CN IM--  0x3C 
  ANDROID_menu             , // MODE==FUNCTION    for -CN IM--  0x3C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN IM--  0x3C
  NONBLE_NOKEY             , // non-BLE shifted   for -CN IM--  0x3C 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN IM--  0x3C 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN IM--  0x3C 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for -CN IM-P  0x3D 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for -CN IM-P  0x3D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -CN IM-P  0x3D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN IM-P  0x3D
  NONBLE_NOKEY             , // non-BLE shifted   for -CN IM-P  0x3D 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN IM-P  0x3D 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN IM-P  0x3D 
},
{
  ANDROID_search           , // MODE==ALPHA       for -CN IMR-  0x3E 
  ANDROID_search           , // MODE==NUMSYM      for -CN IMR-  0x3E 
  ANDROID_search           , // MODE==FUNCTION    for -CN IMR-  0x3E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN IMR-  0x3E
  NONBLE_NOKEY             , // non-BLE shifted   for -CN IMR-  0x3E 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN IMR-  0x3E 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN IMR-  0x3E 
},
{
  HID_KEY_NUM_LOCK         , // MODE==ALPHA       for -CN IMRP  0x3F 
  HID_KEY_NUM_LOCK         , // MODE==NUMSYM      for -CN IMRP  0x3F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for -CN IMRP  0x3F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for -CN IMRP  0x3F
  NONBLE_NOKEY             , // non-BLE shifted   for -CN IMRP  0x3F 
  NONBLE_NOKEY             , // non-BLE unshifted for -CN IMRP  0x3F 
  NONBLE_NOKEY             , // non-BLE numsymed  for -CN IMRP  0x3F 
},
{
  MOD_LSHIFT               , // MODE==ALPHA       for F-- ----  0x40 
  MOD_LSHIFT               , // MODE==NUMSYM      for F-- ----  0x40 
  MOD_LSHIFT               , // MODE==FUNCTION    for F-- ----  0x40 
  BLEMOUSE_SHORTER         , // MODE==MOUSE       for F-- ----  0x40
  MOD_LSHIFT               , // non-BLE shifted   for F-- ----  0x40 
  MOD_LSHIFT               , // non-BLE unshifted for F-- ----  0x40 
  MOD_LSHIFT               , // non-BLE numsymed  for F-- ----  0x40 
},
{
  HID_KEY_RETURN           , // MODE==ALPHA       for F-- ---P  0x41 
  HID_KEY_ENTER            , // MODE==NUMSYM      for F-- ---P  0x41 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- ---P  0x41 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- ---P  0x41
  '\n'                     , // non-BLE shifted   for F-- ---P  0x41 
  '\n'                     , // non-BLE unshifted for F-- ---P  0x41 
  '\n'                     , // non-BLE numsymed  for F-- ---P  0x41 
},
{
  HID_KEY_RIGHT_ARROW      , // MODE==ALPHA       for F-- --R-  0x42 
  HID_KEY_RIGHT_ARROW      , // MODE==NUMSYM      for F-- --R-  0x42 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- --R-  0x42 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- --R-  0x42
  NONBLE_RIGHTARR          , // non-BLE shifted   for F-- --R-  0x42 
  NONBLE_RIGHTARR          , // non-BLE unshifted for F-- --R-  0x42 
  NONBLE_RIGHTARR          , // non-BLE numsymed  for F-- --R-  0x42 
},
{
  HID_KEY_DOWN_ARROW       , // MODE==ALPHA       for F-- --RP  0x43 
  HID_KEY_DOWN_ARROW       , // MODE==NUMSYM      for F-- --RP  0x43 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- --RP  0x43 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- --RP  0x43
  NONBLE_DOWNARR           , // non-BLE shifted   for F-- --RP  0x43 
  NONBLE_DOWNARR           , // non-BLE unshifted for F-- --RP  0x43 
  NONBLE_DOWNARR           , // non-BLE numsymed  for F-- --RP  0x43 
},
{
  HID_KEY_DELETE           , // MODE==ALPHA       for F-- -M--  0x44 
  HID_KEY_DELETE           , // MODE==NUMSYM      for F-- -M--  0x44 
  HID_KEY_DELETE           , // MODE==FUNCTION    for F-- -M--  0x44 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- -M--  0x44
  NONBLE_BACKSPACE         , // non-BLE shifted   for F-- -M--  0x44 
  NONBLE_BACKSPACE         , // non-BLE unshifted for F-- -M--  0x44 
  NONBLE_BACKSPACE         , // non-BLE numsymed  for F-- -M--  0x44 
},
{
  HID_KEY_PRNT_SCREEN      , // MODE==ALPHA       for F-- -M-P  0x45 
  HID_KEY_PRNT_SCREEN      , // MODE==NUMSYM      for F-- -M-P  0x45 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- -M-P  0x45 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- -M-P  0x45
  NONBLE_NOKEY             , // non-BLE shifted   for F-- -M-P  0x45 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- -M-P  0x45 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- -M-P  0x45 
},
{
  HID_KEY_DELETE_FWD       , // MODE==ALPHA       for F-- -MR-  0x46 
  HID_KEY_DELETE_FWD       , // MODE==NUMSYM      for F-- -MR-  0x46 
  MEDIA_playpause          , // MODE==FUNCTION    for F-- -MR-  0x46 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- -MR-  0x46
  NONBLE_NOKEY             , // non-BLE shifted   for F-- -MR-  0x46 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- -MR-  0x46 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- -MR-  0x46 
},
{
  HID_KEY_PAGE_DOWN        , // MODE==ALPHA       for F-- -MRP  0x47 
  HID_KEY_PAGE_DOWN        , // MODE==NUMSYM      for F-- -MRP  0x47 
  MEDIA_next               , // MODE==FUNCTION    for F-- -MRP  0x47 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- -MRP  0x47
  NONBLE_NOKEY             , // non-BLE shifted   for F-- -MRP  0x47 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- -MRP  0x47 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- -MRP  0x47 
},
{
  HID_KEY_LEFT_ARROW       , // MODE==ALPHA       for F-- I---  0x48 
  HID_KEY_LEFT_ARROW       , // MODE==NUMSYM      for F-- I---  0x48 
  MEDIA_previous           , // MODE==FUNCTION    for F-- I---  0x48 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- I---  0x48
  NONBLE_LEFTARR           , // non-BLE shifted   for F-- I---  0x48 
  NONBLE_LEFTARR           , // non-BLE unshifted for F-- I---  0x48 
  NONBLE_LEFTARR           , // non-BLE numsymed  for F-- I---  0x48 
},
{
  HID_KEY_END              , // MODE==ALPHA       for F-- I--P  0x49 
  HID_KEY_END              , // MODE==NUMSYM      for F-- I--P  0x49 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- I--P  0x49 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- I--P  0x49
  NONBLE_NOKEY             , // non-BLE shifted   for F-- I--P  0x49 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- I--P  0x49 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- I--P  0x49 
},
{
  HID_KEY_TAB              , // MODE==ALPHA       for F-- I-R-  0x4A 
  HID_KEY_TAB              , // MODE==NUMSYM      for F-- I-R-  0x4A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- I-R-  0x4A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- I-R-  0x4A
  NONBLE_NOKEY             , // non-BLE shifted   for F-- I-R-  0x4A 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- I-R-  0x4A 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- I-R-  0x4A 
},
{
  HID_KEY_HOME             , // MODE==ALPHA       for F-- I-RP  0x4B 
  HID_KEY_HOME             , // MODE==NUMSYM      for F-- I-RP  0x4B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- I-RP  0x4B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- I-RP  0x4B
  NONBLE_NOKEY             , // non-BLE shifted   for F-- I-RP  0x4B 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- I-RP  0x4B 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- I-RP  0x4B 
},
{
  HID_KEY_UP_ARROW         , // MODE==ALPHA       for F-- IM--  0x4C 
  HID_KEY_UP_ARROW         , // MODE==NUMSYM      for F-- IM--  0x4C 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- IM--  0x4C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- IM--  0x4C
  NONBLE_UPARR             , // non-BLE shifted   for F-- IM--  0x4C 
  NONBLE_UPARR             , // non-BLE unshifted for F-- IM--  0x4C 
  NONBLE_UPARR             , // non-BLE numsymed  for F-- IM--  0x4C 
},
{
  HID_KEY_SCROLL_LOCK      , // MODE==ALPHA       for F-- IM-P  0x4D 
  HID_KEY_SCROLL_LOCK      , // MODE==NUMSYM      for F-- IM-P  0x4D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- IM-P  0x4D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- IM-P  0x4D
  NONBLE_NOKEY             , // non-BLE shifted   for F-- IM-P  0x4D 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- IM-P  0x4D 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- IM-P  0x4D 
},
{
  HID_KEY_PAGE_UP          , // MODE==ALPHA       for F-- IMR-  0x4E 
  HID_KEY_PAGE_UP          , // MODE==NUMSYM      for F-- IMR-  0x4E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- IMR-  0x4E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- IMR-  0x4E
  NONBLE_NOKEY             , // non-BLE shifted   for F-- IMR-  0x4E 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- IMR-  0x4E 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- IMR-  0x4E 
},
{
  HID_KEY_CAPS_LOCK        , // MODE==ALPHA       for F-- IMRP  0x4F 
  HID_KEY_CAPS_LOCK        , // MODE==NUMSYM      for F-- IMRP  0x4F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-- IMRP  0x4F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-- IMRP  0x4F
  NONBLE_NOKEY             , // non-BLE shifted   for F-- IMRP  0x4F 
  NONBLE_NOKEY             , // non-BLE unshifted for F-- IMRP  0x4F 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-- IMRP  0x4F 
},
{
  HID_KEY_PAUSE            , // MODE==ALPHA       for F-N ----  0x50 
  HID_KEY_PAUSE            , // MODE==NUMSYM      for F-N ----  0x50 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N ----  0x50 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N ----  0x50
  NONBLE_NOKEY             , // non-BLE shifted   for F-N ----  0x50 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N ----  0x50 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N ----  0x50 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N ---P  0x51 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N ---P  0x51 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N ---P  0x51 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N ---P  0x51
  NONBLE_NOKEY             , // non-BLE shifted   for F-N ---P  0x51 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N ---P  0x51 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N ---P  0x51 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N --R-  0x52 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N --R-  0x52 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N --R-  0x52 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N --R-  0x52
  NONBLE_NOKEY             , // non-BLE shifted   for F-N --R-  0x52 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N --R-  0x52 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N --R-  0x52 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N --RP  0x53 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N --RP  0x53 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N --RP  0x53 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N --RP  0x53
  NONBLE_NOKEY             , // non-BLE shifted   for F-N --RP  0x53 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N --RP  0x53 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N --RP  0x53 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N -M--  0x54 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N -M--  0x54 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N -M--  0x54 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N -M--  0x54
  NONBLE_NOKEY             , // non-BLE shifted   for F-N -M--  0x54 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N -M--  0x54 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N -M--  0x54 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N -M-P  0x55 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N -M-P  0x55 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N -M-P  0x55 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N -M-P  0x55
  NONBLE_NOKEY             , // non-BLE shifted   for F-N -M-P  0x55 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N -M-P  0x55 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N -M-P  0x55 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N -MR-  0x56 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N -MR-  0x56 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N -MR-  0x56 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N -MR-  0x56
  NONBLE_NOKEY             , // non-BLE shifted   for F-N -MR-  0x56 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N -MR-  0x56 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N -MR-  0x56 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N -MRP  0x57 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N -MRP  0x57 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N -MRP  0x57 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N -MRP  0x57
  NONBLE_NOKEY             , // non-BLE shifted   for F-N -MRP  0x57 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N -MRP  0x57 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N -MRP  0x57 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N I---  0x58 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N I---  0x58 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N I---  0x58 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N I---  0x58
  NONBLE_NOKEY             , // non-BLE shifted   for F-N I---  0x58 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N I---  0x58 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N I---  0x58 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N I--P  0x59 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N I--P  0x59 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N I--P  0x59 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N I--P  0x59
  NONBLE_NOKEY             , // non-BLE shifted   for F-N I--P  0x59 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N I--P  0x59 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N I--P  0x59 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N I-R-  0x5A 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N I-R-  0x5A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N I-R-  0x5A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N I-R-  0x5A
  NONBLE_NOKEY             , // non-BLE shifted   for F-N I-R-  0x5A 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N I-R-  0x5A 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N I-R-  0x5A 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N I-RP  0x5B 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N I-RP  0x5B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N I-RP  0x5B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N I-RP  0x5B
  NONBLE_NOKEY             , // non-BLE shifted   for F-N I-RP  0x5B 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N I-RP  0x5B 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N I-RP  0x5B 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N IM--  0x5C 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N IM--  0x5C 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N IM--  0x5C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N IM--  0x5C
  NONBLE_NOKEY             , // non-BLE shifted   for F-N IM--  0x5C 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N IM--  0x5C 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N IM--  0x5C 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N IM-P  0x5D 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N IM-P  0x5D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N IM-P  0x5D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N IM-P  0x5D
  NONBLE_NOKEY             , // non-BLE shifted   for F-N IM-P  0x5D 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N IM-P  0x5D 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N IM-P  0x5D 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N IMR-  0x5E 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N IMR-  0x5E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N IMR-  0x5E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N IMR-  0x5E
  NONBLE_NOKEY             , // non-BLE shifted   for F-N IMR-  0x5E 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N IMR-  0x5E 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N IMR-  0x5E 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for F-N IMRP  0x5F 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for F-N IMRP  0x5F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for F-N IMRP  0x5F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for F-N IMRP  0x5F
  NONBLE_NOKEY             , // non-BLE shifted   for F-N IMRP  0x5F 
  NONBLE_NOKEY             , // non-BLE unshifted for F-N IMRP  0x5F 
  NONBLE_NOKEY             , // non-BLE numsymed  for F-N IMRP  0x5F 
},
{
  MOD_RSHIFT               , // MODE==ALPHA       for FC- ----  0x60 
  MOD_RSHIFT               , // MODE==NUMSYM      for FC- ----  0x60 
  MOD_RSHIFT               , // MODE==FUNCTION    for FC- ----  0x60 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- ----  0x60
  MOD_RSHIFT               , // non-BLE shifted   for FC- ----  0x60 
  MOD_RSHIFT               , // non-BLE unshifted for FC- ----  0x60 
  MOD_RSHIFT               , // non-BLE numsymed  for FC- ----  0x60 
},
{
  HID_KEY_ENTER            , // MODE==ALPHA       for FC- ---P  0x61 
  HID_KEY_ENTER            , // MODE==NUMSYM      for FC- ---P  0x61 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- ---P  0x61 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- ---P  0x61
  NONBLE_NOKEY             , // non-BLE shifted   for FC- ---P  0x61 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- ---P  0x61 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- ---P  0x61 
},
{
  HID_KEYPAD_6             , // MODE==ALPHA       for FC- --R-  0x62 
  HID_KEYPAD_6             , // MODE==NUMSYM      for FC- --R-  0x62 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- --R-  0x62 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- --R-  0x62
  NONBLE_NOKEY             , // non-BLE shifted   for FC- --R-  0x62 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- --R-  0x62 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- --R-  0x62 
},
{
  MEDIA_volup              , // MODE==ALPHA       for FC- --RP  0x63 
  HID_KEYPAD_2             , // MODE==NUMSYM      for FC- --RP  0x63 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- --RP  0x63 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- --RP  0x63
  NONBLE_NOKEY             , // non-BLE shifted   for FC- --RP  0x63 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- --RP  0x63 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- --RP  0x63 
},
{
  MEDIA_stop               , // MODE==ALPHA       for FC- -M--  0x64 
  HID_KEYPAD_5             , // MODE==NUMSYM      for FC- -M--  0x64 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- -M--  0x64 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- -M--  0x64
  NONBLE_NOKEY             , // non-BLE shifted   for FC- -M--  0x64 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- -M--  0x64 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- -M--  0x64 
},
{
  HID_KEY_MULTIPLY         , // MODE==ALPHA       for FC- -M-P  0x65 
  HID_KEY_MULTIPLY         , // MODE==NUMSYM      for FC- -M-P  0x65 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- -M-P  0x65 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- -M-P  0x65
  NONBLE_NOKEY             , // non-BLE shifted   for FC- -M-P  0x65 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- -M-P  0x65 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- -M-P  0x65 
},
{
  MEDIA_playpause          , // MODE==ALPHA       for FC- -MR-  0x66 
  HID_KEYPAD_DOT           , // MODE==NUMSYM      for FC- -MR-  0x66 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- -MR-  0x66 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- -MR-  0x66
  NONBLE_NOKEY             , // non-BLE shifted   for FC- -MR-  0x66 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- -MR-  0x66 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- -MR-  0x66 
},
{
  MEDIA_next               , // MODE==ALPHA       for FC- -MRP  0x67 
  HID_KEYPAD_3             , // MODE==NUMSYM      for FC- -MRP  0x67 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- -MRP  0x67 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- -MRP  0x67
  NONBLE_NOKEY             , // non-BLE shifted   for FC- -MRP  0x67 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- -MRP  0x67 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- -MRP  0x67 
},
{
  HID_KEYPAD_4             , // MODE==ALPHA       for FC- I---  0x68 
  HID_KEYPAD_4             , // MODE==NUMSYM      for FC- I---  0x68 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- I---  0x68 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- I---  0x68
  NONBLE_NOKEY             , // non-BLE shifted   for FC- I---  0x68 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- I---  0x68 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- I---  0x68 
},
{
  MEDIA_previous           , // MODE==ALPHA       for FC- I--P  0x69 
  HID_KEYPAD_1             , // MODE==NUMSYM      for FC- I--P  0x69 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- I--P  0x69 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- I--P  0x69
  NONBLE_NOKEY             , // non-BLE shifted   for FC- I--P  0x69 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- I--P  0x69 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- I--P  0x69 
},
{
  HID_KEY_SUBTRACT         , // MODE==ALPHA       for FC- I-R-  0x6A 
  HID_KEY_SUBTRACT         , // MODE==NUMSYM      for FC- I-R-  0x6A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- I-R-  0x6A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- I-R-  0x6A
  NONBLE_NOKEY             , // non-BLE shifted   for FC- I-R-  0x6A 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- I-R-  0x6A 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- I-R-  0x6A 
},
{
  HID_KEYPAD_7             , // MODE==ALPHA       for FC- I-RP  0x6B 
  HID_KEYPAD_7             , // MODE==NUMSYM      for FC- I-RP  0x6B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- I-RP  0x6B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- I-RP  0x6B
  NONBLE_NOKEY             , // non-BLE shifted   for FC- I-RP  0x6B 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- I-RP  0x6B 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- I-RP  0x6B 
},
{
  MEDIA_voldn              , // MODE==ALPHA       for FC- IM--  0x6C 
  HID_KEYPAD_8             , // MODE==NUMSYM      for FC- IM--  0x6C 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- IM--  0x6C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- IM--  0x6C
  NONBLE_NOKEY             , // non-BLE shifted   for FC- IM--  0x6C 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- IM--  0x6C 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- IM--  0x6C 
},
{
  HID_KEY_DIVIDE           , // MODE==ALPHA       for FC- IM-P  0x6D 
  HID_KEY_DIVIDE           , // MODE==NUMSYM      for FC- IM-P  0x6D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- IM-P  0x6D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- IM-P  0x6D
  NONBLE_NOKEY             , // non-BLE shifted   for FC- IM-P  0x6D 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- IM-P  0x6D 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- IM-P  0x6D 
},
{
  MEDIA_previous           , // MODE==ALPHA       for FC- IMR-  0x6E 
  HID_KEYPAD_9             , // MODE==NUMSYM      for FC- IMR-  0x6E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- IMR-  0x6E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- IMR-  0x6E
  NONBLE_NOKEY             , // non-BLE shifted   for FC- IMR-  0x6E 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- IMR-  0x6E 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- IMR-  0x6E 
},
{
  HID_KEYPAD_0             , // MODE==ALPHA       for FC- IMRP  0x6F 
  HID_KEYPAD_0             , // MODE==NUMSYM      for FC- IMRP  0x6F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FC- IMRP  0x6F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FC- IMRP  0x6F
  NONBLE_NOKEY             , // non-BLE shifted   for FC- IMRP  0x6F 
  NONBLE_NOKEY             , // non-BLE unshifted for FC- IMRP  0x6F 
  NONBLE_NOKEY             , // non-BLE numsymed  for FC- IMRP  0x6F 
},
{
  MODE_MRESET              , // MODE==ALPHA       for FCN ----  0x70 
  MODE_RESET               , // MODE==NUMSYM      for FCN ----  0x70 
  MODE_RESET               , // MODE==FUNCTION    for FCN ----  0x70 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN ----  0x70
  MODE_MRESET              , // non-BLE shifted   for FCN ----  0x70 
  MODE_MRESET              , // non-BLE unshifted for FCN ----  0x70 
  MODE_RESET               , // non-BLE numsymed  for FCN ----  0x70 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN ---P  0x71 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN ---P  0x71 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN ---P  0x71 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN ---P  0x71
  NONBLE_NOKEY             , // non-BLE shifted   for FCN ---P  0x71 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN ---P  0x71 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN ---P  0x71 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN --R-  0x72 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN --R-  0x72 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN --R-  0x72 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN --R-  0x72
  NONBLE_NOKEY             , // non-BLE shifted   for FCN --R-  0x72 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN --R-  0x72 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN --R-  0x72 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN --RP  0x73 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN --RP  0x73 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN --RP  0x73 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN --RP  0x73
  NONBLE_NOKEY             , // non-BLE shifted   for FCN --RP  0x73 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN --RP  0x73 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN --RP  0x73 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN -M--  0x74 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN -M--  0x74 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN -M--  0x74 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN -M--  0x74
  NONBLE_NOKEY             , // non-BLE shifted   for FCN -M--  0x74 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN -M--  0x74 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN -M--  0x74 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN -M-P  0x75 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN -M-P  0x75 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN -M-P  0x75 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN -M-P  0x75
  NONBLE_NOKEY             , // non-BLE shifted   for FCN -M-P  0x75 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN -M-P  0x75 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN -M-P  0x75 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN -MR-  0x76 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN -MR-  0x76 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN -MR-  0x76 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN -MR-  0x76
  NONBLE_NOKEY             , // non-BLE shifted   for FCN -MR-  0x76 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN -MR-  0x76 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN -MR-  0x76 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN -MRP  0x77 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN -MRP  0x77 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN -MRP  0x77 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN -MRP  0x77
  NONBLE_NOKEY             , // non-BLE shifted   for FCN -MRP  0x77 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN -MRP  0x77 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN -MRP  0x77 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN I---  0x78 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN I---  0x78 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN I---  0x78 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN I---  0x78
  NONBLE_NOKEY             , // non-BLE shifted   for FCN I---  0x78 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN I---  0x78 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN I---  0x78 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN I--P  0x79 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN I--P  0x79 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN I--P  0x79 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN I--P  0x79
  NONBLE_NOKEY             , // non-BLE shifted   for FCN I--P  0x79 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN I--P  0x79 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN I--P  0x79 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN I-R-  0x7A 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN I-R-  0x7A 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN I-R-  0x7A 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN I-R-  0x7A
  NONBLE_NOKEY             , // non-BLE shifted   for FCN I-R-  0x7A 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN I-R-  0x7A 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN I-R-  0x7A 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN I-RP  0x7B 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN I-RP  0x7B 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN I-RP  0x7B 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN I-RP  0x7B
  NONBLE_NOKEY             , // non-BLE shifted   for FCN I-RP  0x7B 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN I-RP  0x7B 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN I-RP  0x7B 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN IM--  0x7C 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN IM--  0x7C 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN IM--  0x7C 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN IM--  0x7C
  NONBLE_NOKEY             , // non-BLE shifted   for FCN IM--  0x7C 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN IM--  0x7C 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN IM--  0x7C 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN IM-P  0x7D 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN IM-P  0x7D 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN IM-P  0x7D 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN IM-P  0x7D
  NONBLE_NOKEY             , // non-BLE shifted   for FCN IM-P  0x7D 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN IM-P  0x7D 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN IM-P  0x7D 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN IMR-  0x7E 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN IMR-  0x7E 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN IMR-  0x7E 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN IMR-  0x7E
  NONBLE_NOKEY             , // non-BLE shifted   for FCN IMR-  0x7E 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN IMR-  0x7E 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN IMR-  0x7E 
},
{
  HID_KEY_RESERVED         , // MODE==ALPHA       for FCN IMRP  0x7F 
  HID_KEY_RESERVED         , // MODE==NUMSYM      for FCN IMRP  0x7F 
  HID_KEY_RESERVED         , // MODE==FUNCTION    for FCN IMRP  0x7F 
  HID_KEY_RESERVED         , // MODE==MOUSE       for FCN IMRP  0x7F
  NONBLE_NOKEY             , // non-BLE shifted   for FCN IMRP  0x7F 
  NONBLE_NOKEY             , // non-BLE unshifted for FCN IMRP  0x7F 
  NONBLE_NOKEY             , // non-BLE numsymed  for FCN IMRP  0x7F 
},
}; // }}}

// end ChordMappings.h
