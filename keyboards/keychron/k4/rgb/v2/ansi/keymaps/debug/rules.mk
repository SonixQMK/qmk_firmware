CONSOLE_ENABLE = yes

# Console adds a few hundred bytes of static RAM; trim ChibiOS stacks to
# keep the 8kB SRAM budget (defaults are 0x600/0x300 for SN32F248B).
USE_PROCESS_STACKSIZE = 0x400
USE_EXCEPTIONS_STACKSIZE = 0x200
