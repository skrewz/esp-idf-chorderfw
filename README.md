# esp-idf-chorderfw

A firmware developed for [s-chorder](https://github.com/skrewz/s-chorder).

This is heavily based off of [nopnop2002's esp-idf-st7789](https://github.com/nopnop2002/esp-idf-st7789), with much of the keyboard/mouse bits borrowed from [asterics's ESP32 keyboard project](https://github.com/asterics/esp32_mouse_keyboard). It's a bit of a work in progress. Pull requests welcome!


## What does it do?

Features:

* Connects to the compile-time configured wifi upon boot.
* Boots up in "note taking mode," where one can submit notes to the compile-time configured URL using an enter key press.
* Pressing both `N` and `C` (near thumb and centre thumb) while hitting `R` (ring) puts it into BLE keyboard mode. Fairly comprehensive chord set including control characters. [chords.md](https://github.com/skrewz/s-chorder/blob/master/chords.md) contains a reference.
* Pressing both `N` and `C` while hitting `M` (middle) puts it into BLE Mouse mode. Vim-style movement commands apply.
* Pressing `N` and `C` while hitting `I` (index) puts it into deep sleep. Kind of unnecessary, though—the thing times out and drops into deep sleep after some amount of inactivity.


## What should it do?

* Some amount of code cleanup is required. This is a proof of concept which is good enough for my purposes at this point. Push me to do it, if you start using this.
* Mouse mode is annoying. It seems almost natural, but isn't quite. Plus, it doesn't support dragging motions, nor mouse buttons beyond the first one.
* Deep sleep mode isn't perfect. Mine drains its battery over a couple of days. It's supposed to be possible to get down to 5uA, which should be lengthy, but clearly that's not the case yet.
* Vibration/alert mode could be used to warn when one is making a suboptimal chord transition.
  * E.g. going from an A (`C+IMR`) to an E (`IMR`) can be done without releasing `IMR` to cause the A, and then releasing e.g. `I` while holding `MR` and re-pressing and releasing `I` to get the E.
  * The fingers of `MR` don't need to move during this at all, and if one is going for an S (`MRP`) afterwards, one can continue this trick.
* Arguably, one should be able to make use of the ESP32's non-BLE bluetooth keyboard mode. They're quite distinct protocols.
* Offline storage for note-taking, for use while away from WiFi? "Keep re-submitting them until 200 is returned?"
