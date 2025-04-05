PROJECT = ToggleTimerLED
BUILD_DIR = build

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m0 -mthumb -std=gnu99 -Wall -O2 \
  -D__CMSIS_GENERIC \
  -D__CORTEX_M0 \
  -D__ARM_ARCH_6M__ \
  -D__CORE__CM0 \
  -D__STATIC_INLINE="static inline" \
  -ILibrary/CMSIS/Include \
  -ILibrary/StdDriver/Include \
  -ILibrary/Device/Nuvoton/NUC100Series/Include \
  -ILibrary/NUC1xx-LB_002/Include \
  -I. \
  -ILibrary/NUC100Series/Include \
  -ILibrary/NUC1xx/Include \
  -ILibrary/Nu-LB-NUC140/Include \
  -ILibrary/SmartcardLib/Include \
  -ILibrary/USB/Include \
  -ISampleCode/USB/Smpl_UVC\
  -ISampleCode/USB/Smpl_HID\
  -ISampleCode/USB/Smpl_UAC\
  -ISampleCode/USB/Smpl_UAC_HID\
  -ISampleCode/USB/Smpl_UDC\
  -Isrc\
  -ISampleCode/USB/Smpl_VCOM

LDFLAGS = -Tgcc_arm.ld -nostartfiles -Wl,--gc-sections -Wl,-e,Reset_Handler

# Explicit list of source files
C_SOURCES = \
  src/main.c \
  Library/NUC100Series/Source/system_NUC100Series.c \
  Library/StdDriver/Source/clk.c \
  Library/StdDriver/Source/uart.c \
  Library/StdDriver/Source/gpio.c \
  Library/StdDriver/Source/sys.c \
  Library/Nu-LB-NUC140/Source/SYS_init.c \
  Library/StdDriver/Source/timer.c

ASM_SOURCES = Library/NUC100Series/Source/GCC/startup_NUC100Series.S

# Derived
C_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJS = $(patsubst %.S, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
OBJS = $(C_OBJS) $(ASM_OBJS)

.PHONY: all clean

all: $(BUILD_DIR)/$(PROJECT).hex

$(BUILD_DIR)/$(PROJECT).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

