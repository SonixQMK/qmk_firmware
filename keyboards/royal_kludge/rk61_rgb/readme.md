# RK61 RGB

![RK61 RGB](https://i.pinimg.com/originals/01/e2/1f/01e21f1ba56526a0f2fe5047b6d25f11.png)

Royal Kludge RK61 non-hotswap rgb edition, original PID 0x24F.

Tested on the Royal Kludge RK61 non-hotswap RGB dual-mode edition, original PID 0x24F

- MCU: HFD2201KBA (rebrand SN32F248BF)
- PCB: RK61-JX-RGB-HFD; REV: 01; 2019/08/13
- Wired only, no bluetooth support.

Based on previous work by [euwbah](https://github.com/euwbah). Added VIA 3.0 support.

Keyboard Maintainer: [fpb](https://github.com/fpb)

For the white-backlit non-rgb version of the RK61, check out [Diff-fusion's repo](https://github.com/Diff-fusion/qmk_firmware)


Note: keymap differs slightly from stock firmware for FN combinations

See [keymap.c](keymaps/default/keymap.c), and refer to the [list
of QMK keycodes](https://beta.docs.qmk.fm/using-qmk/simple-keycodes/keycodes).

-----------------


Make example for this keyboard (after setting up your build environment):

    make royal_kludge/rk61_rgb:default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader:

- Physical reset: short the two contacts to the left of the space bar switch with a wire/paperclip. Accessible from the front.
- Layout (after loading QMK): FN + Left Control
