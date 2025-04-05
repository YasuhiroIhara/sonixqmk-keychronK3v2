# project specific files
SRC = rand.c

# MCU name
MCU = SN32F268F

# Build Options
#   comment out to disable the options.
#
MAGIC_ENABLE = yes
MAGIC_KEYCODE_ENABLE = yes
BOOTMAGIC_ENABLE = yes 	# Virtual DIP switch configuration
EXTRAKEY_ENABLE = yes   	# Audio control and System control
NKRO_ENABLE = yes       	# USB Nkey Rollover
DIP_SWITCH_ENABLE = yes

# Custom Key and LED matrix handling
CUSTOM_MATRIX = no
RGB_MATRIX_ENABLE = no

# Reduce code size
USE_PROCESS_STACKSIZE    = 0x1E0
USE_EXCEPTIONS_STACKSIZE = 0xE0