# Bare-metal build for STM32F446RE (NUCLEO-F446RE)
#   make                         -> builds examples/spi_loopback
#   make EXAMPLE=spi_master_slave
#   make flash                   -> needs st-flash (stlink-tools) or use STM32CubeProgrammer
#   make clean

EXAMPLE ?= spi_loopback
BUILD   := build/$(EXAMPLE)
TARGET  := $(BUILD)/$(EXAMPLE)

PREFIX  ?= arm-none-eabi-
CC      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE    := $(PREFIX)size

MCU     := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

SRCS := examples/$(EXAMPLE)/main.c \
        $(wildcard drivers/src/*.c) \
        startup/startup_stm32f446xx.c \
        startup/syscalls.c

INCS := -Idrivers/inc

CFLAGS  := $(MCU) -std=gnu11 -O1 -g3 -Wall -Wextra -Wno-unused-parameter \
           -ffunction-sections -fdata-sections $(INCS) -DSTM32F446xx $(CFLAGS_EXTRA)
LDFLAGS := $(MCU) -T linker/STM32F446RETX_FLASH.ld -Wl,--gc-sections \
           -Wl,-Map=$(TARGET).map -Wl,--print-memory-usage \
           --specs=nano.specs --specs=nosys.specs

OBJS := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

all: $(TARGET).elf $(TARGET).bin $(TARGET).hex

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

flash: $(TARGET).bin
	st-flash --reset write $< 0x08000000

clean:
	rm -rf build

.PHONY: all flash clean
