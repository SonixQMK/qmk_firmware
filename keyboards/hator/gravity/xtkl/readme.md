# gravity 

![gravity x tkl](https://i.imgur.com/WKBMvUh.png)

*A short description of the keyboard/project*

* Keyboard Maintainer: [Alex](https://github.com/AlexHtr)
* Hardware Supported: Wired mode, side led enabled
* Hardware Availability: https://hator.com/keyboards/gravity-x-tkl/

Make firmware for this keyboard (after setting up your build environment, 2 types of layouts available):

    qmk compile -kb hator/gravity/xtkl -km via
or
    qmk compile -kb hator/gravity/xtkl -km default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [SonixQMK instructions](https://sonixqmk.github.io//SonixDocs/install/) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

Use [Sonix Keyboard Flasher] (https://github.com/SonixQMK/sonix-flasher/) to flash your firmware. Offset is 0x00.

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Escape) and plug in the keyboard
* **Physical reset button**: You can locate the boot pins under the Spacebar.
* **Keycode in layout**: Press Fn1+ESC to enter the bootloader mode.
