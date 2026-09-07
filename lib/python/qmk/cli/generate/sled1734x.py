"""Used by the make system to generate the SLED1734X register binding from an LED binding.
"""
import struct
from enum import Enum
from typing import TypedDict

from milc import cli

from qmk.commands import dump_lines
from qmk.constants import GENERATED_HEADER_C_LIKE, GPL2_HEADER_C_LIKE
from qmk.path import normpath


# sled1734x_color_channel_t
class ColorChannel(Enum):
    RED = "RED"
    GREEN = "GREEN"
    BLUE = "BLUE"


# sled1734x_led_t
class Led(TypedDict):
    driver: int
    r: int
    g: int
    b: int


# sled1734x_register_t
class Register(TypedDict):
    led_index: int
    color_channel: ColorChannel


def make_binding(leds: list[Led]):
    drivers: dict[int, dict[int, Register]] = {}

    def set_pin(driver: int, pin: int, i: int, color: ColorChannel):
        if pin in drivers[driver]:
            cli.log.warning("Register %s for driver %d reassigned", pin, driver)
        drivers[driver][pin] = {'led_index': i, 'color_channel': color}

    for i, led in enumerate(leds):
        driver, red_pin, green_pin, blue_pin = led["driver"], led["r"], led["g"], led["b"]

        if driver not in drivers:
            drivers[driver] = {}

        set_pin(driver, red_pin, i, ColorChannel.RED)
        set_pin(driver, green_pin, i, ColorChannel.GREEN)
        set_pin(driver, blue_pin, i, ColorChannel.BLUE)

    return drivers


def make_file(drivers: dict[int, dict[int, Register]]):
    lines = [
        GPL2_HEADER_C_LIKE,
        GENERATED_HEADER_C_LIKE,
        '',
        '#include "progmem.h"',
        '#include "rgb_matrix_types.h"',
        '#include "sled1734x.h"',
        '',
        'const sled1734x_register_t PROGMEM g_sled1734x_registers[SLED1734X_DRIVER_COUNT][SLED1734X_PWM_REGISTER_COUNT] = {',
    ]

    for driver, registers in sorted(drivers.items()):
        lines.append(f'    [{driver}] = {{')
        lines.append('        [0 ... SLED1734X_PWM_REGISTER_COUNT - 1] = { .led_index = NO_LED, .color_channel = RED },')

        for name, register in sorted(registers.items()):
            led_index, color_channel = register["led_index"], register["color_channel"]
            lines.append(f'        [{name}] = {{.led_index = {led_index}, .color_channel = {color_channel.value}}},')

        lines.append('    },')

    lines.append('};')

    return lines


def parse_leds_array(file_path: str):
    # The shape of our sled1734x_led_t - 4 "uint8_t"s, in order:
    # driver, r, g, b
    struct_format = "<BBBB"
    element_size = struct.calcsize(struct_format)

    leds: list[Led] = []
    with open(file_path, 'rb') as f:
        while True:
            chunk = f.read(element_size)

            chunk_len = len(chunk)
            if not chunk:
                break
            if chunk_len < element_size:
                cli.log.warning("%d bytes left over in input file", chunk_len)
                break

            driver, r, g, b = struct.unpack(struct_format, chunk)
            # This field is defined as only two bits, cut off the unused padding
            driver = driver & 0x03

            leds.append({
                "driver": driver,
                "r": r,
                "g": g,
                "b": b,
            })

    return leds


@cli.argument('-o', '--output', arg_only=True, type=normpath, help='File to write to')
@cli.argument('-q', '--quiet', arg_only=True, action='store_true', help='Quiet mode, only output error messages')
@cli.argument('--obj', arg_only=True, required=True, help='Keyboard object file, containing g_sled1734x_leds.')
@cli.subcommand('Used by the make system to generate the SLED1734X register binding.', hidden=True)
def generate_sled1734x(cli):
    leds = parse_leds_array(cli.args.obj)

    binding = make_binding(leds)

    lines = make_file(binding)

    dump_lines(cli.args.output, lines, cli.args.quiet)
