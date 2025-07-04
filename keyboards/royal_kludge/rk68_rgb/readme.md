# RK68 RGB

![RK68 RGB](https://i.ibb.co/bLYNDjb/IMG-20210321-025644.jpg)

Royal Kludge RK68 hotswap dual mode, rgb edition, original PID 0x24F. Later updated to 0x8016.

- MCU: HFD2201KBA (rebrand SN32F248BF)
- PCB: RK61-JX-RGB-HFD; REV: 01; 2020/11/04
- Wired only, no bluetooth support.

Updated to work with SonixQMK/qmk_firmware:sn32_develop branch. Based on previous work by [euwbah](https://github.com/euwbah). Added VIA 3.0 support.

Keyboard Maintainer: [fpb](https://github.com/fpb)

For the white-backlit non-rgb version of the RK68, check out [silasb' repo](https://github.com/silasb/qmk_firmware)


Note: keymap differs slightly from stock firmware for FN combinations

See [keymap.c](keymaps/default/keymap.c), and refer to the [list
of QMK keycodes](https://beta.docs.qmk.fm/using-qmk/simple-keycodes/keycodes).

Keyboard will always start in Mac mode but you can toggle between Windows (FN+A) and Mac (FN+S) layouts. Function keys work as function keys.

Building instructions
-----------------


Make example for this keyboard (after setting up your build environment):

    $ make royal_kludge/rk68_rgb:all

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader:

- Physical reset: short the two pins on the underside of the pcb, as indicated below. You need to remove some keycaps and 6 screws to be able to flip the PCB.
- Layout (after loading QMK): FN + CTRL

![Bootloader mode](https://i.ibb.co/BT7sj31/bootmode.jpg)

## Keymaps

Keysmaps are identical in both VIA and default layouts. VIA version has one additional empty layer to customize.

![Layer 0](https://i.ibb.co/9wXbcmM/layer0.jpg)
![Layer 1](https://i.ibb.co/q9T33QJ/layer1.jpg)

## VIA Json file

You need [this json file](./keymaps/via/rk68-rgb_via.json) to add your keyboard at https://usevia.app. Go to settings, Show Design TAB, Go to Design Tab and Load Draft Definition.

## Final result

Here is a picture of my RK68 with Mac style keycaps...

![RK68 RGB running QMK/VIA firmware](https://i.ibb.co/HCrk3D3/IMG-20231118-133253.jpg)