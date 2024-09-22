# SN32F2XX Driver {#sn32f2xx-driver}

This driver provides support for PWM driven RGB LED matrix on the Sonix SN32F2xx series of chips.

## Hardware Configuration {#hardware-configuration}

The most common way to configure the LED wiring is on a `3 ( R/B/G channel sinks) x 1 ( actively driven PWM line)` matrix, controlled by transistors through GPIO pins.

```
    Default configuration example

    COLS key / led
    SS8050 transistors NPN driven low
    base      - GPIO
    collector - LED Col pins
    emitter   - VDD

    VDD     GPIO
    (E)     (B)
     |  PNP  |
     |_______|
         |
         |
        (C)
        LED

    ROWS RGB
    SS8550 transistors PNP driven high
    base      - GPIO
    collector - LED RGB row pins
    emitter   - GND

        LED
        (C)
         |
         |
      _______
     |  NPN  |
     |       |
    (B)     (E)
    GPIO    GND

```

The driver has provisions to either use hardware PWM, or bitbang if the pins configured do not have available PWM mode.

## Usage {#usage}

In most cases, the SN32F2XX driver code is automatically included if you are using the [RGB Matrix](../features/rgb_matrix) feature with the `sn32f2xx` driver set, and you would use that API instead.

However, if you need to use the driver standalone, add the following to your `rules.mk`:

```make
COMMON_VPATH += $(DRIVER_PATH)/led
SRC += sn32f2xx.c
```

You can then call the SN32F2XX API by including `sn32f2xx.h` in your code.

## Basic Configuration {#basic-configuration}

Add the following to your `config.h`:

|Define                             |Default                           |Description                                                                                         |
|-----------------------------------|----------------------------------|----------------------------------------------------------------------------------------------------|
|`SN32F2XX_RGB_MATRIX_ROW_PINS`     |*Not defined*                     |The GPIO pins connected to the RGB sink triplets on every row of the LEDs in the matrix<sup>1</sup> |
|`SN32F2XX_RGB_MATRIX_COL_PINS`     |`MATRIX_COL_PINS`                 |The GPIO pins connected to the LED columns on every column of the LEDs in the matrix<sup>2</sup>    |
|`SN32F2XX_RGB_MATRIX_COLS`         |`MATRIX_COLS`                     |Number of LED columns in the LED matrix                                                             |
|`SN32F2XX_RGB_MATRIX_ROWS`         |`MATRIX_ROWS`                     |Number of LED rows in the LED matrix                                                                |
|`SN32F2XX_RGB_MATRIX_ROW_CHANNELS` |`3`                               |The number of available color channels for each LED                                                 |
|`SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL` |`SN32F2XX_PWM_OUTPUT_ACTIVE_LOW`  |The logic level that enables PWM output. Depends on transistor controlling PWM                      |
|`SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL` |`SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH` |The logic level that enables RGB output. Depends on transistor controlling RGB sinks                |
|`SN32F2XX_PWM_DIRECTION`           |`DIODE_DIRECTION`                 |The scanning logic direction for the LED matrix                                                     |
|`SN32F2XX_PWM_CONTROL`             |`HARDWARE_PWM`                    |Toggles hardware or software control for the PWM                                                    |
|`SN32F2XX_LED_OUTPUT_LUMINOSITY_R` |`1`                               |Luminosity correction factor for the Red channel                                                    |
|`SN32F2XX_LED_OUTPUT_LUMINOSITY_G` |`1`                               |Luminosity correction factor for the Green channel                                                  |
|`SN32F2XX_LED_OUTPUT_LUMINOSITY_B` |`1`                               |Luminosity correction factor for the Blue channel                                                   |

Notes:

1. The currently supported RGB byte order is `R/B/G` for each row triplet. Example config for 3 rows `(R0, B0, G0, R1, B1, G1, R2, B2, G2)`
2. If not user defined, it assumes shared matrix

## Shared Matrix {#shared-matrix}

The driver can share LED matrix wiring with the key matrix, specifically columns. It takes over the key scanning routines and carefully times it to avoid key chatter and LED artifacts. Please note that key scan cycles are affected, although the result is a snappy key scan.


## ChibiOS/ARM Configuration {#arm-configuration}

The following defines apply only to ARM devices:

### PWM Driver {#arm-pwm-driver}

Depending on the ChibiOS board configuration, you may need to enable PWM at the keyboard level. For SN32, this would look like:

`halconf.h`:
```c
#define HAL_USE_PWM TRUE
```
`mcuconf.h`:
```c
#undef SN32_PWM_USE_CT16B1
#define SN32_PWM_USE_CT16B1 TRUE
```

and additionally configure the CT peripheral

`mcuconf.h`:
```c
#undef SN32_PWM_NO_RESET
#define SN32_PWM_NO_RESET TRUE
```

The driver uses the CT timer to handle PWM, even if software PWM is selected.

## API {#api}

### `void sn32f2xx_set_color(int index, uint8_t red, uint8_t green, uint8_t blue)` {#api-sn32f2xx-set-color}

Set the color of a single LED (RGB driver only).

#### Arguments {#api-sn32f2xx-set-color-arguments}

 - `int index`
   The LED index.
 - `uint8_t red`
   The red value to set.
 - `uint8_t green`
   The green value to set.
 - `uint8_t blue`
   The blue value to set.

---

### `void sn32f2xx_set_color_all(uint8_t red, uint8_t green, uint8_t blue)` {#api-sn32f2xx-set-color-all}

Set the color of all LEDs (RGB driver only).

#### Arguments {#api-sn32f2xx-set-color-all-arguments}

 - `uint8_t red`
   The red value to set.
 - `uint8_t green`
   The green value to set.
 - `uint8_t blue`
   The blue value to set.
