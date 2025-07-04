# Keychron C2 RGB

![Keychron C2](https://i.imgur.com/QtV3Kk3.jpg)

A customizable Full-Size ANSI keyboard.

* Keyboard Maintainer(s): [SonixQMK](https://github.com/SonixQMK), [IsaacDynamo](https://github.com/IsaacDynamo), [noldevin](https://github.com/noldevin), [vjdato21](https://github.com/vjdato21), [aaronsb](https://github.com/aaronsb), [JAO1988](https://github.com/JAO1988)
* Hardware Supported: Keychron C2 RGB
* Hardware Availability: [Keychron](https://www.keychron.com/products/keychron-c2-wired-mechanical-keyboard)

Make example for this keyboard (after setting up your build environment):

    make keychron/c2/rgb:default

## How to Flash
Flashing example for this keyboard:
1. If your keyboard currently has stock firmware installed, put your keyboard first into bootloader by shorting the boot pins found under the spacebar before plugging in your keyboard to the PC. Otherwise, press `Fn + Esc` to put your keyboard into bootloader.
1. Download and run [Sonix Flasher](https://github.com/SonixQMK/sonix-flasher/releases/latest).
1. In Sonix Flasher, select `SN32F24X` under 'Device'. And select `0x00` as the qmk offset.
1. Lastly, click 'Flash QMK...' and locate the compiled firmware.

* * *
# Keymapping
### Windows Mode
<details>

Without Fn | With Fn
---------- | -------
F1 | Decrease PC Brightness
F2 | Increase PC Brightness
F3 | Task View
F4 | File Explorer
F5 | Decrease KB Brightness
F6 | Increase KB Brightness
F7 | Previous Track
F8 | Play/Pause Track
F9 | Next Track
F10 | Mute
F11 | Decrease Volume
F12 | Increase Volume
Print Screen | Snip & Sketch
Cortana / Microsoft Teams | None
Change RGB | Toggle RGB
⊞ | Windows Key Lock Toggle
Home | NKRO Toggle
↑ | Increase RGB Hue
← | Decrease RGB Saturation
↓ | Decrease RGB Hue
→ | Increase RGB Saturation

</details>

### Mac Mode
<details>

Without Fn | With Fn
---------- | -------
F1 | Decrease PC Brightness
F2 | Increase PC Brightness
F3 | Mission Control
F4 | Finder
F5 | Decrease KB Brightness
F6 | Increase KB Brightness
F7 | Previous Track
F8 | Play/Pause Track
F9 | Next Track
F10 | Mute
F11 | Decrease Volume
F12 | Increase Volume
Take a screenshot (whole screen) | Take a screenshot (specific area)
Siri (hold down) | None
Change RGB | Toggle RGB
Home | NKRO Toggle
↑ | Increase RGB Hue
← | Decrease RGB Saturation
↓ | Decrease RGB Hue
→ | Increase RGB Saturation

</details>

* * *

For more information and more detailed flashing instructions, please visit https://github.com/CanUnesi/QMK-on-K6#readme

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
