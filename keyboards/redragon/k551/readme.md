# Redragon K551RGB MITRA

[K551RGB](https://imgur.com/a/WeNJ6gF)

* Keyboard Maintainer: [Samuel Lafontaine](https://github.com/exoticpubis)
* Based on K556 by: [Adam Honse](https://github.com/CalcProgrammer1)
* Hardware Supported: SN32F248BF

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb redragon/k551 -km default

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
