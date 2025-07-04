# sn32f290 splittest

This is the splittest example for the sn32f290 MCU.
To trigger keypress, short together pins _B10_ and _B1_.

## Keyboard Configuration

The type of communication (bitbang, serial (half/full duplex)) is set by the keymap `config.h` file with variables with `SPLIT_` prefix.
So this example uses:

-   `config.h` to set which SDI to use for serial. Either define `USE_SDI0`, `USE_SDI1` , `USE_SDI2 `or `USE_SDI3` before the `#ifdef` that configures the default case when none of these variable is defined.
-   `post_config.h` for defining the communication pins between the halves once the keymap is configured.

On your keyboard, you most likely will only use `config.h` because you'd have already chosen your communication scheme: serial type, SDI, pins.

## Wiring

### Switches

-   Add switches to both sn32f290 across B10 and B1 pins

### Handedness

Have a look at the [handedness documentation](https://docs.qmk.fm/#/feature_split_keyboard?id=setting-handedness)

-   Add pull-up resistor to left side between VCC and A7
-   Add pull-down resistors to right side between GND and A7

### Send power to the other half

-   Connect the following pins on both sides together: GND, VCC

### Halves data connection

Choose one of the connection type

#### serial - bitbang

-   Connect the B8 pins on both sides together
-   Note that bitbang functionality requires strict timings. Currently not supported.

#### serial - usart half duplex

Check the [documentation](https://docs.qmk.fm/#/serial_driver?id=usart-half-duplex) to determine the pull-up resistor.

-   To use the default usart (UART0),
    -   Connect the A10, A11 pins on both sides together and to add a pull-up resistor on one of these pins
    -   Define USE_SDI0 in config.h
-   To use the second usart (UART1),
    -   Connect the B8, B9 pins on both sides together and to add a pull-up resistor on one of these pins
    -   Define USE_SDI1 in config.h
-   To use the third usart (UART2),
    -   Connect the A1, A2 pins on both sides together and to add a pull-up resistor on one of these pins
    -   Define USE_SDI2 in config.h
-   To use the fourth usart (UART3),
    -   Connect the D12, D13 pins on both sides together and to add a pull-up resistor on one of these pins
    -   Define USE_SDI3 in config.h

#### serial - usart full duplex

-   To use the default usart (UART0),
    -   Connect the pins on A10, A11 one to the other on each side
    -   Define USE_SDI0 in config.h
-   To use the second usart (UART1),
    -   Connect the pins B8, B9 one to the other on each side
    -   Define USE_SDI1 in config.h
-   To use the third usart2 (UART2) pins,
    -   Connect the pins A1, A2 one to the other on each side
    -   Define USE_SDI2 in config.h
-   To use the fourth usart3 (UART3) pins,
    -   Connect the pins D12, D13 one to the other on each side
    -   Define USE_SDI3 in config.h