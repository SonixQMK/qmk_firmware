# project specific files
SRC += gmmk.c

# MCU name
MCU = SN32F268F

# Build Options
#   comment out to disable the options.
#
BACKLIGHT_ENABLE = no
MAGIC_ENABLE = no
BOOTMAGIC_ENABLE = yes  # Virtual DIP switch configuration
MOUSEKEY_ENABLE = no    # Mouse keys
EXTRAKEY_ENABLE = yes   # Audio control and System control
SLEEP_LED_ENABLE = no   # Breathing sleep LED during USB suspend
NKRO_ENABLE = yes
KEYBOARD_SHARED_EP = yes
CONSOLE_ENABLE = no
SERIAL_LINK_ENABLE = no
WAIT_FOR_USB = yes
CUSTOM_MATRIX = no
LED_MATRIX_ENABLE = no
LED_MATRIX_DRIVER = custom
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = custom
RAW_ENABLE = yes

# ENCODER_ENABLE = yes
# OPENRGB_ENABLE = yes

USE_EXCEPTIONS_STACKSIZE = 0xE0
USE_PROCESS_STACKSIZE = 0x1E0

DEFAULT_FOLDER = gmmk/full/rev3
