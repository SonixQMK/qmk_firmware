# Keychron K3V2 RGB Optical ISO

*A 75% keyboard sold by keychron*

* Keyboard Maintainer: [Swiftb0y](https://github.com/Swiftb0y/) (based on work of macaholik and KeijoMika on a previous branch)
* Hardware Supported: *Keychron K3V2 ISO RGB, MCU: SN32F248BF*
* Hardware Availability: *https://www.keychron.com/collections/keychron-k-series-keyboard/products/keychron-k3-wireless-mechanical-keyboard*

Compile example for this keyboard (after setting up your build environment):

    qmk compile -kb keychron/k3/rgb/v2/optical_iso -km default

Compile example for this keyboard with VIA (after setting up your build environment):

    qmk compile -kb keychron/k3/rgb/v2/optical_iso -km via

This keyboard must be flashed with an external [GUI](https://github.com/SonixQMK/sonix-flasher) or [CLI](https://github.com/SonixQMK/SonixFlasherC)


See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Physical Bootloader Pins**: Remove the backcover of the Keyboard and short pin next to MCU labeled BOOT to ground.
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available. In the default keymap Fn+Esc.
