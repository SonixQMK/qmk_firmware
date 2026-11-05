# PixArt PAR2801. custom keeps bluetooth.c's weak hooks and lets k530_bt.c
# supply the real ones.
BLUETOOTH_ENABLE = yes
BLUETOOTH_DRIVER = custom
SRC += k530_bt.c
