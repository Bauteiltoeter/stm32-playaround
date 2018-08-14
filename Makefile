BUILDDIR = build

DEVICE = stm32/CMSIS/Device/ST/STM32F4xx/
CORE = stm32/CMSIS/
PERIPH = stm32/STM32F4xx_StdPeriph_Driver
DISCOVERY = stm32/discovery
USB = stm32/usb
CONTROLLER = STM32F429_439xx

#SOURCES += $(DISCOVERY)/src/stm32f3_discovery.c

SOURCES += $(PERIPH)/src/stm32f4xx_gpio.c \
		   $(PERIPH)/src/stm32f4xx_i2c.c \
		   $(PERIPH)/src/stm32f4xx_rcc.c \
		   $(PERIPH)/src/stm32f4xx_spi.c \
		   $(PERIPH)/src/stm32f4xx_exti.c \
		   $(PERIPH)/src/stm32f4xx_dma.c \
		   $(PERIPH)/src/stm32f4xx_usart.c \
		   $(PERIPH)/src/stm32f4xx_tim.c \
		   $(PERIPH)/src/misc.c \
		   $(PERIPH)/src/stm32f4xx_syscfg.c 

SOURCES += startup_stm32f4xx.s
SOURCES += system_stm32f4xx.c

SOURCES += main.c LC24.c

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I$(DEVICE)/Include \
			-I$(CORE)/Include \
			-I$(PERIPH)/inc \
			-I$(DISCOVERY)/include \
			-I$(USB)/include \
			-I\

ELF = $(BUILDDIR)/program.elf
HEX = $(BUILDDIR)/program.hex
BIN = $(BUILDDIR)/program.bin

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
 	
CFLAGS  = -O0 -g -Wall -I.\
   -mcpu=cortex-m4 -mthumb \
   -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
   $(INCLUDES) -DUSE_STDPERIPH_DRIVER -D$(CONTROLLER) -DHSE_VALUE=8000000UL

LDSCRIPT = stm32_flash.ld
LDFLAGS += -T$(LDSCRIPT) -mthumb -mcpu=cortex-m4 -nostdlib

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

all: $(BIN)

flash: $(BIN)
	st-flash write $(BIN) 0x8000000

clean:
	rm -rf build
