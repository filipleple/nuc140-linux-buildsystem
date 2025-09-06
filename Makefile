# Makefile for Nuvoton NUC140 (Cortex-M0) — Smpl_SDCard
# Auto-detects driver family and matches the correct Device BSP.

# ---------- Project ----------
PROJECT        ?= Smpl_SDCard
BUILD_DIR      ?= build

# ---------- Layout ----------
APP_ROOT       ?= Smpl_SDcard
BSP_ROOT       ?= Library

# FatFs location (Smpl_SDcard/ff8/src or Smpl_SDcard/ff8)
FATFS_SRC_DIR  := $(if $(wildcard $(APP_ROOT)/ff8/src/ff.c),$(APP_ROOT)/ff8/src,$(APP_ROOT)/ff8)
FATFS_ROOT     := $(FATFS_SRC_DIR)

# BSP families
DEV_NUC1XX     := $(BSP_ROOT)/Device/Nuvoton/NUC1xx
DEV_NUC100     := $(BSP_ROOT)/Device/Nuvoton/NUC100Series

# Common library roots
STDDRIVER_ROOT ?= $(BSP_ROOT)/StdDriver
BOARD_LIB_ROOT ?= $(BSP_ROOT)/NUC1xx-LB_002
NUC1XX_MISC    ?= $(BSP_ROOT)/NUC1xx

# ---------- Toolchain ----------
CC       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
OBJDUMP  := arm-none-eabi-objdump
SIZE     := arm-none-eabi-size

CPUFLAGS := -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
CSTD     := -std=gnu11

DEFS := \
  -D__ARM_ARCH_6M__ \
  -D__CORTEX_M0 \
  -D__CORE__CM0 \
  -D__STATIC_INLINE="static inline"

# ---------- Driver + Device auto-detect ----------
# DEV_FAMILY: auto | NUC1xx | NUC100Series
DEV_FAMILY ?= auto

# Detect driver style:
# - StdDriver(lowercase) => Library/StdDriver/Source/gpio.c exists
# - NUC1xx(UPPERCASE)    => Library/NUC1xx/Source/GPIO.c exists
ifneq ($(wildcard $(STDDRIVER_ROOT)/Source/gpio.c),)
  DRIVER_STYLE := StdDriver(lowercase)
  HAVE_STDDRIVER := 1
endif
ifneq ($(wildcard $(NUC1XX_MISC)/Source/GPIO.c),)
  DRIVER_STYLE_BACKUP := NUC1xx(UPPERCASE)
  HAVE_NUC1XXDRV := 1
endif

# Choose DEV_ROOT based on DEV_FAMILY and what exists.
ifeq ($(DEV_FAMILY),NUC1xx)
  DEV_ROOT := $(DEV_NUC1XX)
else ifeq ($(DEV_FAMILY),NUC100Series)
  DEV_ROOT := $(DEV_NUC100)
else
  # auto: if StdDriver exists, pair with NUC100Series; else use NUC1xx
  ifeq ($(HAVE_STDDRIVER),1)
    DEV_ROOT := $(DEV_NUC100)
  else ifeq ($(HAVE_NUC1XXDRV),1)
    DEV_ROOT := $(DEV_NUC1XX)
  else
    # last resort: first that exists
    DEV_ROOT := $(firstword $(filter-out nonexistent,$(foreach d,$(DEV_NUC1XX) $(DEV_NUC100),$(if $(wildcard $(d)),$(d),nonexistent))))
  endif
endif

ifeq ($(strip $(DEV_ROOT)),)
  $(error Could not find Device/Nuvoton BSP under $(BSP_ROOT))
endif

# ---------- Includes ----------
INCLUDES := \
  -I$(APP_ROOT) \
  -I$(FATFS_ROOT) \
  -I$(BSP_ROOT)/CMSIS/Include \
  -I$(DEV_ROOT)/Include \
  -I$(DEV_ROOT)/Source \
  -I$(BSP_ROOT) \
  -I$(BOARD_LIB_ROOT)/Include

# Add matching driver include path
# For StdDriver(lowercase), headers live under Library/StdDriver/Include and include NUC100Series.h
ifeq ($(HAVE_STDDRIVER),1)
  INCLUDES += -I$(STDDRIVER_ROOT)/Include
else ifeq ($(HAVE_NUC1XXDRV),1)
  # For old NUC1xx drivers, headers like SYS.h live under Library/NUC1xx/Include
  INCLUDES += -I$(NUC1XX_MISC)/Include
endif

CFLAGS := $(CPUFLAGS) $(CSTD) -O2 -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Wno-unused-parameter $(DEFS) $(INCLUDES)

# ---------- Linker ----------
LD_SCRIPT ?= nuc140.ld
LDFLAGS  := -T$(LD_SCRIPT) -Wl,--gc-sections -Wl,-Map,$(BUILD_DIR)/$(PROJECT).map -nostartfiles -Wl,-e,Reset_Handler
LDLIBS   := -Wl,--start-group -lc -lm -lgcc -lnosys -Wl,--end-group

# ---------- Source selection ----------
# Startup/system tied to chosen DEV_ROOT
STARTUP_CANDIDATES := \
  $(DEV_ROOT)/Source/arm/startup_NUC1xx.s \
  $(DEV_ROOT)/Source/GCC/startup_NUC1xx.s \
  $(DEV_ROOT)/Source/GCC/startup_NUC100Series.S

SYSTEM_CANDIDATES := \
  $(DEV_ROOT)/Source/system_NUC1xx.c \
  $(DEV_ROOT)/Source/system_NUC100Series.c

STARTUP := $(firstword $(wildcard $(STARTUP_CANDIDATES)))
SYSTEMC := $(firstword $(wildcard $(SYSTEM_CANDIDATES)))

ifeq ($(strip $(STARTUP)),)
$(error Could not find a GCC-compatible startup file. Looked in: $(STARTUP_CANDIDATES))
endif

ifeq ($(strip $(SYSTEMC)),)
$(error Could not find system_* file. Looked in: $(SYSTEM_CANDIDATES))
endif

# App and FatFs
APP_SRCS := \
  $(APP_ROOT)/main.c \
  $(FATFS_ROOT)/diskio.c \
  $(FATFS_ROOT)/ff.c

# Board support from NUC1xx-LB_002
BOARD_SRCS := \
  $(BOARD_LIB_ROOT)/Source/LCD.c \
  $(BOARD_LIB_ROOT)/Source/SDCard.c \
  $(BOARD_LIB_ROOT)/Source/Scankey.c

# Driver sources (auto-picked)
ifeq ($(HAVE_STDDRIVER),1)
  # StdDriver(lowercase file names)
  STDDRIVER_SRCS := \
    $(STDDRIVER_ROOT)/Source/gpio.c \
    $(STDDRIVER_ROOT)/Source/sys.c \
    $(STDDRIVER_ROOT)/Source/spi.c \
    $(STDDRIVER_ROOT)/Source/i2c.c \
    $(STDDRIVER_ROOT)/Source/i2s.c \
    $(STDDRIVER_ROOT)/Source/uart.c
else ifeq ($(HAVE_NUC1XXDRV),1)
  # NUC1xx(UPPERCASE file names)
  STDDRIVER_SRCS := \
    $(NUC1XX_MISC)/Source/GPIO.c \
    $(NUC1XX_MISC)/Source/SYS.c  \
    $(NUC1XX_MISC)/Source/SPI.c  \
    $(NUC1XX_MISC)/Source/I2C.c  \
    $(NUC1XX_MISC)/Source/I2S.c  \
    $(NUC1XX_MISC)/Source/UART.c
else
  $(error Could not find StdDriver sources in $(STDDRIVER_ROOT)/Source or $(NUC1XX_MISC)/Source)
endif

# Misc Nuvoton (retarget) — lives under NUC1xx tree in both BSPs
NUC_MISC_SRCS := \
  $(NUC1XX_MISC)/Source/retarget.c

C_SOURCES := \
  $(SYSTEMC) \
  $(APP_SRCS) \
  $(BOARD_SRCS) \
  $(STDDRIVER_SRCS) \
  $(NUC_MISC_SRCS)

ASM_SOURCES := $(STARTUP)

# ---------- Derived ----------
ELF := $(BUILD_DIR)/$(PROJECT).elf
HEX := $(BUILD_DIR)/$(PROJECT).hex
BIN := $(BUILD_DIR)/$(PROJECT).bin
LST := $(BUILD_DIR)/$(PROJECT).lst

C_OBJS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJS := $(patsubst %.S,$(BUILD_DIR)/%.o,$(filter %.S,$(ASM_SOURCES))) \
            $(patsubst %.s,$(BUILD_DIR)/%.o,$(filter %.s,$(ASM_SOURCES)))
OBJS := $(C_OBJS) $(ASM_OBJS)

# ---------- Rules ----------
.PHONY: all clean size bin lst info

all: $(HEX) $(BIN) size

$(ELF): $(OBJS) $(LD_SCRIPT) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

lst: $(LST)
$(LST): $(ELF)
	$(OBJDUMP) -d -C -S $< > $@

size: $(ELF)
	$(SIZE) --format=berkeley $(ELF)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Pattern rules with directory creation
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

info:
	@echo "DEV_FAMILY   = $(DEV_FAMILY)"
	@echo "DEV_ROOT     = $(DEV_ROOT)"
	@echo "STARTUP      = $(STARTUP)"
	@echo "SYSTEMC      = $(SYSTEMC)"
	@echo "APP_ROOT     = $(APP_ROOT)"
	@echo "FATFS_ROOT   = $(FATFS_ROOT)"
	@echo "DRIVER_STYLE = $(if $(HAVE_STDDRIVER),StdDriver(lowercase),$(if $(HAVE_NUC1XXDRV),NUC1xx(UPPERCASE),unknown))"
