### SLED1734X Driver {#sled1734x-driver}

I²C 16x16 LED matrix driver by Sonix. Supports a maximum of four drivers, each controlling up to 256 single-color LEDs, or 70 RGB LEDs.


[SLED1734X Datasheet](https://www.sonix.com.tw/files/1/637E852205EF6BC3E050007F01007792)

## Usage {#usage}

The SLED1734X driver code is automatically included if you are using the [RGB Matrix](../features/rgb_matrix) feature with the `sled1734x` driver set, and you would use that API instead.

However, if you need to use the driver standalone, add this to your `rules.mk`:

```make
COMMON_VPATH += $(DRIVER_PATH)/led
SRC += sled1734x.c # For RGB
I2C_DRIVER_REQUIRED = yes
```

## Basic Configuration {#basic-configuration}

Add the following to your `config.h`:

|Define                      |Default      |Description                                         |
|----------------------------|-------------|----------------------------------------------------|
|`SLED1734X_SDB_PIN`        |*Not defined*|The GPIO pin connected to the drivers' shutdown pins|
|`SLED1734X_I2C_TIMEOUT`    |`100`        |The I²C timeout in milliseconds                     |
|`SLED1734X_I2C_PERSISTENCE`|`0`          |The number of times to retry I²C transmissions      |
|`SLED1734X_I2C_ADDRESS_1`  |*Not defined*|The I²C address of driver 0                         |
|`SLED1734X_I2C_ADDRESS_2`  |*Not defined*|The I²C address of driver 1                         |
|`SLED1734X_I2C_ADDRESS_3`  |*Not defined*|The I²C address of driver 2                         |
|`SLED1734X_I2C_ADDRESS_4`  |*Not defined*|The I²C address of driver 3                         |

### I²C Addressing {#i2c-addressing}

The SLED1734X has four possible 7-bit I²C addresses, depending on how the `ADDR` pin is connected.

To configure this, set the `SLED1734X_I2C_ADDRESS_n` defines to one of the following in your `config.h`, where *n* denotes the driver index:

|Define                        |Value |
|------------------------------|------|
|`SLED1734X_I2C_ADDRESS_GND`  |`0x74`|
|`SLED1734X_I2C_ADDRESS_SCL`  |`0x75`|
|`SLED1734X_I2C_ADDRESS_SDA`  |`0x76`|
|`SLED1734X_I2C_ADDRESS_VDDIO`|`0x77`|


## ARM/ChibiOS Configuration {#arm-configuration}

Depending on the ChibiOS board configuration, you may need to [enable and configure I²C](i2c#arm-configuration) at the keyboard level.

## LED Mapping {#led-mapping}

In order to use this driver, each output must be mapped to an LED index, by adding the following to your `<keyboardname>.c`:

```c
const sled1734x_led PROGMEM g_sled1734x_leds[SLED1734X_LED_COUNT] = {
/* Driver
 *   |  R         G         B */
    {0, CA3_D,  CA1_D,  CA2_D},
    // etc...
};
```

In this example, the red, green and blue channels for the first LED index on driver 0 all have their anodes connected to the `D` pin, and their cathodes on the `CA1`, `CA2` and `CA3` pins respectively.

These values correspond to the register indices as shown in the datasheet on page 64.
At the moment, the driver supports MATRIX TYPE 3 only.

## API {#api}

### `struct sled1734x_led_t` {#api-sled1734x-led-t}

Contains the PWM register addresses for a single RGB LED.

#### Members {#api-sled1734x-led-t-members}

 - `uint8_t driver`
   The driver index of the LED, from 0 to 3.
 - `uint8_t r`
   The output PWM register address for the LED's red channel (RGB driver only).
 - `uint8_t g`
   The output PWM register address for the LED's green channel (RGB driver only).
 - `uint8_t b`
   The output PWM register address for the LED's blue channel (RGB driver only).
 - `uint8_t v`
   The output PWM register address for the LED (single-color driver only).

---

### `void sled1734x_init(uint8_t index)` {#api-sled1734x-init}

Initialize the LED driver. This function should be called first.

#### Arguments {#api-sled1734x-init-arguments}

 - `uint8_t index`
   The driver index.

---

### `void sled1734x_write_register(uint8_t index, uint8_t reg, uint8_t data)` {#api-sled1734x-write-register}

Set the value of the given register.

#### Arguments {#api-sled1734x-write-register-arguments}

 - `uint8_t index`
   The driver index.
 - `uint8_t reg`
   The register address.
 - `uint8_t data`
   The value to set.

---

### `void sled1734x_select_page(uint8_t index, uint8_t page)` {#api-sled1734x-select-page}

Change the current page for configuring the LED driver.

#### Arguments {#api-sled1734x-select-page-arguments}

 - `uint8_t index`
   The driver index.
 - `uint8_t page`
   The page number to select.

---

### `void sled1734x_set_color(int index, uint8_t red, uint8_t green, uint8_t blue)` {#api-sled1734x-set-color}

Set the color of a single LED (RGB driver only). This function does not immediately update the LEDs; call `sled1734x_update_pwm_buffers()` after you are finished.

#### Arguments {#api-sled1734x-set-color-arguments}

 - `int index`
   The LED index (ie. the index into the `g_sled1734x_leds` array).
 - `uint8_t red`
   The red value to set.
 - `uint8_t green`
   The green value to set.
 - `uint8_t blue`
   The blue value to set.

---

### `void sled1734x_set_color_all(uint8_t red, uint8_t green, uint8_t blue)` {#api-sled1734x-set-color-all}

Set the color of all LEDs (RGB driver only).

#### Arguments {#api-sled1734x-set-color-all-arguments}

 - `uint8_t red`
   The red value to set.
 - `uint8_t green`
   The green value to set.
 - `uint8_t blue`
   The blue value to set.

---

### `void sled1734x_set_led_control_register(uint8_t index, bool red, bool green, bool blue)` {#api-sled1734x-set-led-control-register-rgb}

Configure the LED control registers for a single LED (RGB driver only). This function does not immediately update the LEDs; call `sled1734x_update_led_control_registers()` after you are finished.

#### Arguments {#api-sled1734x-set-led-control-register-rgb-arguments}

 - `uint8_t index`
   The LED index (ie. the index into the `g_sled1734x_leds` array).
 - `bool red`
   Enable or disable the red channel.
 - `bool green`
   Enable or disable the green channel.
 - `bool blue`
   Enable or disable the blue channel.

---

### `void sled1734x_update_pwm_buffers(uint8_t index)` {#api-sled1734x-update-pwm-buffers}

Flush the PWM values to the LED driver.

#### Arguments {#api-sled1734x-update-pwm-buffers-arguments}

 - `uint8_t index`
   The driver index.

---

### `void sled1734x_update_led_control_registers(uint8_t index)` {#api-sled1734x-update-led-control-registers}

Flush the LED control register values to the LED driver.

#### Arguments {#api-sled1734x-update-led-control-registers-arguments}

 - `uint8_t index`
   The driver index.
