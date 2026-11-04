# Marvo KG938

![Marvo KG938](https://www.marvo.ro/image/cache/catalog/tastaturi/KG938/4-800x800.jpg)

Full-sized mechanical gaming keyboard with per-key RGB backlighting, powered by the eVision VS11K09A (Sonix SN32F248F) microcontroller.

* Keyboard Maintainer: [Pescu](https://github.com/Pescu123)
* Hardware Supported: Marvo KG938 (eVision VS11K09A / Sonix SN32F248F)
* Hardware Availability: [Marvo Romania](https://www.marvo.ro/index.php?route=product/product&product_id=112)

Make example for this keyboard (after setting up your build environment):

    make marvo/kg938:default

Flashing example for this keyboard:

    make marvo/kg938:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (`Escape`) and plug in the keyboard
* **Physical reset button**: Short the boot pins/pads (ISP jumper) on the PCB while plugging in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available

### Flashing Notes

* MCU: SN32F248F (VS11K09A)
* ISP Bootloader VID/PID: `0x0C45:0x7900`
* Flashing Offset: `0x00` (Do NOT use `0x200` or jumploader)
* Flashing tool: [SonixFlasher](https://github.com/SonixQMK/SonixFlasher) or Sonix USB MCU ISP Tool with SN32F248 setting at address `0x00000000`.
  ```sh
  ./sonixflasher -v 0c45/7900 -o 0x00 -f marvo_kg938_default.bin
  ```

## Credits

* Original implementation based on SPCGear GK540 Magna by [HorrorTroll](https://github.com/HorrorTroll).
