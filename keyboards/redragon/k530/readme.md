# Redragon K530 Draconic (RGB)

A 61-key keyboard from Redragon.

* Keyboard Maintainer: [uvera](https://github.com/uvera)
* Hardware Supported: Redragon K530 (VS11K09A-1 / SN32F248BF) with a PixArt PAR2801 radio
* Hardware Availability: [Redragon official website](https://www.redragonzone.com/products/redragonk530)

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb redragon/k530 -km default

Flashing example for this keyboard:

    qmk flash -kb redragon/k530 -km default

`qmk flash` calls `sonixflasher` (https://github.com/SonixQMK/SonixFlasherC). Put the keyboard in the bootloader first (hold Caps, tap Right, or hold Esc while plugging in). The bootloader shows up as `0c45:7040`.

Hold Caps for the function layer. Fn+Esc talks to the radio: tap reconnects, hold for 3 seconds pairs a new host (`Draconic -1`, `-2`, or `-3`, chosen by the BT slot switch). The cable/BT switch selects wired or wireless. Fn+Right enters the bootloader.

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
