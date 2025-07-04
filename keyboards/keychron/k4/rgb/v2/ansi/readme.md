# keychron_K4V2_RGB_ANSI


*A 96% keyboard sold by keychron*

* Keyboard Maintainer: [PythonDeployer](https://github.com/PythonDeployer) (big thanks to the SonixQMK discord)
* Hardware Supported: *Keychron K4V2 RGB, MCU: SN32F248BF*
* Hardware Availability: *https://www.keychron.com/products/keychron-k4-wireless-mechanical-keyboard-version-2*

Compile example for this keyboard (after setting up your build environment):

    qmk compile -kb keychron/k4/rgb/v2/ansi -km default

Compile example for this keyboard with VIA (after setting up your build environment):

    qmk compile -kb keychron/k4/rgb/v2/ansi -km via

This keyboard must be flashed with an external [GUI](https://github.com/SonixQMK/sonix-flasher) or [CLI](https://github.com/SonixQMK/SonixFlasherC)



See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 1 way:

* **Physical Bootloader Pins**: Turn off the keyboard, Using a pair of tweezers or a paperclip, short the 2 pins next to boot and hold the tweezers on the pins while turning the keyboard on, this should put it into the bootloader.

