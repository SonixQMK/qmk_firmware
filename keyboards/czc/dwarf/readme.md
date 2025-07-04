# CZC.Gaming Dwarf

![czc/dwarf](https://i.imgur.com/AV2fSoy.png)

A keyboard sold by a dommestic reatiler CZC.cz from the Czech Republic. It is only produced with an ANSI layout.

* Keyboard Maintainer: [Václav Straka](https://github.com/Vesek) (big thanks to the SonixQMK discord)
* Hardware Supported: CZC.Gaming Dwarf
* Hardware Availability: [czc.cz official store](https://www.czc.cz/czc-gaming-dwarf-herni-klavesnice-kailh-red-cz/288631/produkt)

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb czc/dwarf -km default

This keyboard must be flashed with an external [GUI](https://github.com/SonixQMK/sonix-flasher) or [CLI](https://github.com/SonixQMK/SonixFlasherC)

CLI flashing example for this keyboard (after putting it in bootloader mode):
    
    sonixflasher -v 0c45/7040 -f czc_dwarf_default.bin

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down Esc and plug in the keyboard (doesn't work but is still enabled)
* **Keycode in layout**: Press Fn+Esc which is mapped to `QK_BOOT`
