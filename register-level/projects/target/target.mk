# target.mk — shared STM32H563ZI target build for every module project.
# Each project Makefile sets PROJECT, APP, DEPTH then includes this file.
# Produces build-target/$(PROJECT).hex/.bin — flashable with STM32CubeProgrammer.
TCROOT  := C:/ST/STM32CubeCLT_1.22.0/GNU-tools-for-STM32/bin
CC      := "$(TCROOT)/arm-none-eabi-gcc.exe"
OBJCOPY := "$(TCROOT)/arm-none-eabi-objcopy.exe"
MCUFLAGS := -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard

# DEPTH — relative path from the project dir back up to projects/
DEPTH   ?= ../
# ROOT — the register-level/ directory; TGT — shared target infra dir
ROOT    := $(DEPTH)/..
TGT     := $(DEPTH)/target
CFLAGS  := $(MCUFLAGS) -O1 -Wall -fno-common -g \
           -DSTM32H563xx -DUSE_FULL_LL_DRIVER \
           -I. -I$(TGT) -I$(ROOT)drivers \
           -I$(ROOT)/../Drivers/CMSIS/Device/ST/STM32H5xx/Include \
           -I$(ROOT)/../Drivers/CMSIS/Include \
           -I$(DEPTH)/lib
DEVSRC := $(ROOT)/drivers/devices/dig_in/dig_in.c $(ROOT)/drivers/devices/analog_in/analog_in.c \
          $(ROOT)/drivers/devices/dig_out/dig_out.c $(ROOT)/drivers/devices/pwm_out/pwm_out.c \
          $(ROOT)/drivers/devices/quad_enc/quad_enc.c \
          $(ROOT)/drivers/devices/kit/kit.c $(ROOT)/drivers/devices/pal/devpal.c \
          $(ROOT)/drivers/peripherals/gpio/stm_gpio.c $(ROOT)/drivers/peripherals/adc/stm_adc.c \
          $(ROOT)/drivers/peripherals/tim/stm_tim.c
LIBSRC := $(DEPTH)/lib/blink.c $(DEPTH)/lib/edge_count.c $(DEPTH)/lib/hysteresis.c \
          $(DEPTH)/lib/colorwheel.c $(DEPTH)/lib/sequencer.c $(DEPTH)/lib/peak_detect.c
BOOTSRC := $(TGT)/system_stm32h5xx.c $(TGT)/startup_stm32h563zitx.s
TDIR    := build-target

all: $(TDIR)/$(PROJECT).hex $(TDIR)/$(PROJECT).bin

$(TDIR):
	mkdir -p $(TDIR)

$(TDIR)/$(PROJECT).elf: main.c $(APP).c $(DEVSRC) $(LIBSRC) $(BOOTSRC) | $(TDIR)
	$(CC) $(CFLAGS) -o $@ main.c $(APP).c $(DEVSRC) $(LIBSRC) $(BOOTSRC) \
	      -T$(TGT)/STM32H563ZITX_FLASH.ld -Wl,-Map=$(TDIR)/$(PROJECT).map --specs=nosys.specs

$(TDIR)/$(PROJECT).hex: $(TDIR)/$(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@
$(TDIR)/$(PROJECT).bin: $(TDIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(TDIR)

.PHONY: all clean
