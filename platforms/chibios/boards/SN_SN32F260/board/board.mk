# List of all the board related files.
BOARDSRC = $(CHIBIOS_CONTRIB)/os/hal/boards/SN_SN32F260/board.c

# Required include directories
BOARDINC = $(CHIBIOS_CONTRIB)/os/hal/boards/SN_SN32F260

# Optimize for size
# OPT = s /set by default

# Shave some extra bytes
OPT_DEFS += -DCRT0_AREAS_NUMBER=1

# Some options to reduce RAM usage
USE_LINK_GC = yes
LTO_ENABLE = yes

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
