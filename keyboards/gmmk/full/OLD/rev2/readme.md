# GMMK Full Mechanical Keyboard

Hardware Supported: SN32F268
Revisions
rev1: IS31FL3731    , NOT SUPPORTED, red led color only
rev2: VS12L03A (I2C), SN before SNSS12116xxxxx
rev3: VS12L17A (SPI), SN equal or after SNSS12116xxxxx

Make example for this keyboard (after setting up your build environment):

    to build ANSI keymap: (104 keys)
    make gmmk/full:default
    make gmmk/full/rev2:default
    make gmmk/full/rev3:default
    
    to build ISO keymap: (105 keys)
    make gmmk/full:iso
    make gmmk/full/rev2:iso
    make gmmk/full/rev3:iso

    to build VIA:
    make gmmk/full/rev2:via
    make gmmk/full/rev3:via
    
    For VIA + OpenRGB (WIP),
    https://github.com/gloryhzw/qmk_tool/wiki/VIA-OpenRGB-for-GMMK

PS.
1. FULL would work on TKL. Some rgb effects would got offset issues.
2. Rev2 firmware will work on rev3 without rgb. Hang otherwise.

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions] (https://docs.qmk.fm/#/getting_started_make_guide) for more information. 
Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
