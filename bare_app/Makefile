ALT_DEVICE_FAMILY ?= soc_cv_av
SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
HWLIBS_ROOT = $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib

DEBUG := 1
DEFINES += -DALT_INT_PROVISION_CPU_COUNT=2
DEFINES += -DUSE_SERIAL_PORT
# BUFFERED_SERIAL_PORT - буферизированный вывод (асинхронный), иначе - синхронный; использовать совместно с USE_SERIAL_PORT.
#DEFINES += -DBUFFERED_SERIAL_PORT
DEFINES += -DALT_FPGA_ENABLE_DMA_SUPPORT=1
DEFINES += -DDISCR_FREQ_DEF_1200

VPATH := src src/hwlib src/fatfs
INCLUDES := -Isrc -Isrc/hwlib -Isrc/fatfs -I$(HWLIBS_ROOT)/include -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY)

LINKER_SCRIPT := scatter.scat

CROSS_COMPILE := arm-altera-eabi-
CC := armcc
AS := armasm
LD := armlink
OC := $(CROSS_COMPILE)objcopy

CFLAGS   := --strict --diag_suppress=9931 --cpu=Cortex-A9 --no_unaligned_access --c99 $(DEFINES) $(INCLUDES) -D$(ALT_DEVICE_FAMILY)
ASMFLAGS := --diag_error=warning --diag_suppress=9931 --cpu=Cortex-A9 --no_unaligned_access
LDFLAGS  := --strict --diag_error=warning --diag_suppress=9931 --entry=alt_interrupt_vector --cpu=Cortex-A9 --no_unaligned_access --scatter=$(LINKER_SCRIPT)

ifdef DEBUG
CFLAGS   += -g -O0 -DDEBUG=1
ASMFLAGS += -g
else
CFLAGS   += -O3 -DSTANDALONE
endif

ELF      := main_prj.axf
SRC_DIR  := src/
C_SRC := $(wildcard $(SRC_DIR)*.c) $(wildcard $(SRC_DIR)*/*.c)
OBJ_DIR  := obj/
OBJ      := $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(C_SRC))) \
            $(OBJ_DIR)alt_interrupt_armcc.o

RM := rm -rf
MKDIR = mkdir -p

.PHONY: all
all: print directories $(ELF)

.PHONY: clean
clean:
	$(RM) $(ELF) $(OBJ) $(HWLIBS_SRC)

directories:
	${MKDIR} ${OBJ_DIR}

print:
	echo

# $< The name of the first prerequisite. If the target got its recipe from an implicit rule, this will be the first prerequisite added by the implicit rule
# $@ The file name of the target of the rule.
$(OBJ_DIR)%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)alt_interrupt_armcc.o: alt_interrupt_armcc.s
	$(AS) $(ASMFLAGS) $< -o $@

$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@
	$(OC) --gap-fill=0xff -O binary $(ELF) $(patsubst %.axf,%.bin,$(ELF))
	mkimage.exe -A arm -T firmware -C none -O u-boot -a 0x30000000 -e 0x40 -n "main_prj" -d $(patsubst %.axf,%.bin,$(ELF)) $(patsubst %.axf,%.img,$(ELF))