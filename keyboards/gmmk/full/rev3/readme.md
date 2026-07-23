# GMMK Full Mechanical Keyboard

[GMMK Full Mechanical Keyboard](https://www.gloriousgaming.com/en-es/pages/guide-gmmk1-fullsize)

* Keyboard Maintainer: [N/A](N/A)
* Based on K556 by: [N/A](N/A)
* Hardware Supported: SN32F268
* Revisions
* rev1: IS31FL3731    , NOT SUPPORTED, red led color only
* rev2: VS12L03A (I2C), SN before SNSS12116xxxxx
* rev3: VS12L17A (SPI), SN equal or after SNSS12116xxxxx

Only the rev3 ISO keymap code has been ported from sn32_master.

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb gmmk/full/rev3 -km iso

For flashing this keyboard:

    use sonixflasher: https://github.com/SonixQMK/SonixFlasherC/releases
    or sonix-flasher (GUI): https://github.com/SonixQMK/sonix-flasher/releases

See the [build environment setup](https://sonixqmk.github.io/qmk_docs/newbs_getting_started) and the [make instructions](https://sonixqmk.github.io/qmk_docs/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://sonixqmk.github.io/qmk_docs/newbs).

## Bootloader

Enter the bootloader:

    -Use one of the 2 sonix-flasher to enter bootloader from original firmware.
    -Bootmagic reset: Hold down the key at (0,0) in the matrix (usually the top   left key or Escape) and plug in the keyboard
    -Physical reset button: Briefly press the button on the back of the PCB - some may have pads you must short instead
    -Keycode in layout: Press the key mapped to QK_BOOT if it is available
