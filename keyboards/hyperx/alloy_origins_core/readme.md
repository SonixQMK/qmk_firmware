# coreorigin

![coreorigin](https://imgur.com/a/7St30pv)

The HyperX Alloy Origins Core is HyperX's tkl board in their origins lineup.

* Keyboard Maintainer: [Nekomancer](https://github.com/nekomancer834)
* Hardware Supported: HyperX Alloy Origins Core
* Hardware Availability: [*Amazon*](https://www.amazon.com/HyperX-Alloy-Origins-Core-Customization/dp/B07YMHGP86)

Make example for this keyboard (after setting up your build environment):

    make HyperX/alloyoriginscore:default

Flashing example for this keyboard:

    make HyperX/alloyoriginscore:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Escape) and plug in the keyboard
* **Physical reset button**: Briefly connect tp282 on the back of the PCB to the usb connector shield
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
