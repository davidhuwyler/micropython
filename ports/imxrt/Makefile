# Select the board to build for: if not given on the command line,
# then default to PYBV10.
BOARD ?= PYBV10
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

include ../../py/mkenv.mk
-include mpconfigport.mk
include boards/$(BOARD)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
override QSTR_DEFS += qstrdefsport.h $(BUILD)/pins_qstr.h $(BUILD)/modmcu_qstr.h

# directory containing scripts to be frozen as bytecode
#FROZEN_MPY_DIR ?= modules

# include py core make definitions
include $(TOP)/py/py.mk

LD_DIR=boards
HAL_DIR=mcusdk
MCU_SERIES_UPPER = $(shell echo $(MCU_SERIES) | tr '[:lower:]' '[:upper:]')
USBDEV_DIR=usb_misc
#USBHOST_DIR=usbhost
FATFS_DIR=lib/oofatfs
DFU=$(TOP)/tools/dfu.py
# may need to prefix dfu-util with sudo
USE_PYDFU ?= 1
PYDFU ?= $(TOP)/tools/pydfu.py
DFU_UTIL ?= dfu-util
DEVICE=0483:df11
STFLASH ?= st-flash
OPENOCD ?= openocd
OPENOCD_CONFIG ?= boards/openocd_stm32f4.cfg
STARTUP_FILE ?= boards/startup_mimxrt1052.c

CROSS_COMPILE = arm-none-eabi-

INC += -I.
INC += -I..
INC += -I$(BUILD)
INC += -I$(USBDEV_DIR)
#INC += -I$(TOP_DIR)/micropython
INC += -I$(TOP)/py
#INC += -I$(TOP)/lib
#INC += -I$(TOP)/lib/oofatfs
#INC += -I$(TOP)/lib/mp-readline
INC += -I$(BUILD)
#INC += -I$(TOP_DIR)/micropython/lib/cmsis/inc
#INC += -I$(TOP_DIR)/micropython/lib/cmsis/inc
#INC += -I$(USBHOST_DIR)
INC += -I$(HAL_DIR)
INC += -I$(HAL_DIR)/drivers
INC += -I$(HAL_DIR)/utilities
#INC += -I$(HAL_DIR)/CMSIS
#INC += -I$(HAL_DIR)/CMSIS_driver
INC += -I$(HAL_DIR)/component/uart
INC += -I$(HAL_DIR)/component/serial_manager
INC += -I$(HAL_DIR)/component/serial_manager/usb_cdc_adapter
INC += -I$(HAL_DIR)/usb/phy
INC += -I$(HAL_DIR)/usb/include
INC += -I$(HAL_DIR)/usb/device/include
INC += -I$(HAL_DIR)/usb/device/class
INC += -I$(HAL_DIR)/usb/device/source
INC += -I$(HAL_DIR)/usb/device/source/ehci
INC += -I$(HAL_DIR)/usb/device/source/khci
INC += -I$(HAL_DIR)/osa
INC += -I$(HAL_DIR)/video
INC += -I$(HAL_DIR)/sdmmc
INC += -Iflegftl
INC += -Iusb_misc

#INC += -I$(TOP)/omv
#INC += -I$(TOP)/omv/py
#INC += -I$(TOP)/omv/img
#INC += -I$(TOP)/omv/nn


# Options for particular MCU series
#CFLAGS_CORTEX_M7 = -DCPU_MIMXRT1052CVL5B -DCPU_MIMXRT1052CVL5B_cm7 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DXIP_EXTERNAL_FLASH=1 -DXIP_BOOT_HEADER_ENABLE=1 -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -fstack-usage
CFLAGS_CORTEX_M7 = -DCPU_MIMXRT1052CVL5B -DCPU_MIMXRT1052CVL5B_cm7 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -fstack-usage


CFLAGS += $(INC)
CFLAGS += $(CFLAGS_CORTEX_M7)

CFLAGS += -DXIP_BOOT_HEADER_ENABLE=1
CFLAGS += -DXIP_BOOT_HEADER_DCD_ENABLE=1
CFLAGS += -DXIP_EXTERNAL_FLASH=1

CFLAGS += -Iboards/$(BOARD)
CFLAGS += -DMICROPY_HW_VTOR=$(TEXT0_ADDR)
#Rocky Flags
#CFLAGS += -DUSB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE=1
#CFLAGS += -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1

CFLAGS += -DSDRAM_MPU_INIT=1

ifeq ($(MICROPY_FLOAT_IMPL),double)
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_DOUBLE
else
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
endif

LDFLAGS = -nostdlib -L $(LD_DIR) $(addprefix -T,$(LD_FILES)) -Map=$(@:.elf=.map) --cref
LIBS = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# Remove uncalled code from the final image.
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += --gc-sections

SRC_LIB = $(addprefix lib/,\
	libc/string0.c \
	oofatfs/ff.c \
	oofatfs/option/unicode.c \
	mp-readline/readline.c \
	netutils/netutils.c \
	timeutils/timeutils.c \
	utils/pyexec.c \
	utils/interrupt_char.c \
	utils/sys_stdio_mphal.c \
	utils/printf.c \
	)

ifeq ($(MICROPY_FLOAT_IMPL),double)
SRC_LIBM = $(addprefix lib/libm_dbl/,\
	__cos.c \
	__expo2.c \
	__fpclassify.c \
	__rem_pio2.c \
	__rem_pio2_large.c \
	__signbit.c \
	__sin.c \
	__tan.c \
	acos.c \
	acosh.c \
	asin.c \
	asinh.c \
	atan.c \
	atan2.c \
	atanh.c \
	ceil.c \
	cos.c \
	cosh.c \
	erf.c \
	exp.c \
	expm1.c \
	floor.c \
	fmod.c \
	frexp.c \
	ldexp.c \
	lgamma.c \
	log.c \
	log10.c \
	log1p.c \
	modf.c \
	nearbyint.c \
	pow.c \
	rint.c \
	scalbn.c \
	sin.c \
	sinh.c \
	sqrt.c \
	tan.c \
	tanh.c \
	tgamma.c \
	trunc.c \
	)
else
SRC_LIBM = $(addprefix lib/libm/,\
	math.c \
	roundf.c \
	thumb_vfp_sqrtf.c \
	acoshf.c \
	asinfacosf.c \
	asinhf.c \
	atan2f.c \
	atanf.c \
	atanhf.c \
	ef_rem_pio2.c \
	erf_lgamma.c \
	fmodf.c \
	kf_cos.c \
	kf_rem_pio2.c \
	kf_sin.c \
	kf_tan.c \
	log1pf.c \
	nearbyintf.c \
	sf_cos.c \
	sf_erf.c \
	sf_frexp.c \
	sf_ldexp.c \
	sf_modf.c \
	sf_sin.c \
	sf_tan.c \
	wf_lgamma.c \
	wf_tgamma.c \
	)
endif

EXTMOD_SRC_C = $(addprefix extmod/,\
	modonewire.c \
        )

DRIVERS_SRC_C = $(addprefix drivers/,\
	bus/softspi.c \
	bus/softqspi.c \
	memory/spiflash.c \
	dht/dht.c \
	)

SRC_C = \
	main.c \
	adc.c \
	bufhelper.c \
	dcmc.c \
	irq.c \
	isr.c \
	fatfs_port.c \
	flash_pgm.c \
	flash_hyper.c \
	flash_QuadSpi.c \
	hal_wrapper.c \
	pin.c \
	pin_defs_mcu.c \
	pin_named_pins.c \
	pybstdio.c \
	pybthread.c \
	pwm.c \
	mphalport.c \
	mpthreadport.c \
	pybthread.c \
	modmcu.c \
	modmachine.c \
	modpyb.c \
	moduos.c \
	modnetwork.c \
	modusocket.c \
	modutime.c \
	rpm.c \
	rng.c \
	rtc.c \
	srpm.c \
	tmr.c \
	qtimer.c \
	systick.c \
	servo.c \
	storage.c \
	sdcard.c \
	gccollect.c \
	led.c \
	flegftl/flegftl.c \
	flash.c \
	help.c \
	uart.c \
	usrsw.c \
	pendsv.c \
	usb_app.c \
	uniqueID.c \
	usbd_msc_storage.c \
	$(STARTUP_FILE) \
	$(wildcard boards/$(BOARD)/*.c)
	
SRC_O = \
	resethandler.o \
	gchelper.o \
	
SRC_UTIL = 	$(addprefix $(HAL_DIR)/utilities/, \
	fsl_str.c \
	fsl_debug_console.c \
	)

SRC_HAL = $(addprefix $(HAL_DIR)/drivers/, \
	fsl_adc.c \
	fsl_clock.c \
	fsl_cache.c \
	fsl_flexspi.c \
	fsl_snvs_hp.c \
	fsl_trng.c \
	fsl_qtmr.c \
	fsl_gpio.c \
	fsl_lpspi.c \
	fsl_lpuart.c\
	fsl_usdhc.c \
	fsl_elcdif.c \
	fsl_csi.c \
	fsl_lpi2c.c \
	)
	
SRC_HAL += $(addprefix $(HAL_DIR)/, \
	usb/device/source/usb_device_dci.c \
	usb/device/source/ehci/usb_device_ehci.c \
	usb/device/class/usb_device_cdc_acm.c \
	usb/device/class/usb_device_class.c \
	usb/device/source/usb_device_ch9.c \
	utilities/fls_debug_console.c \
	component/serial_manager/serial_manager.c \
	component/serial_manager/serial_port_uart.c \
	component/uart/lpuart_adapter.c \
	usb/phy/usb_phy.c \
	osa/usb_osa_bm.c \
	sdmmc/fsl_sd.c \
	sdmmc/fsl_sdmmc_common.c \
	sdmmc/port_usdhc_irq/fsl_sdmmc_host.c \
	sdmmc/port_usdhc_irq/fsl_sdmmc_event.c \
	video/fsl_csi_camera_adapter.c \
	video/fsl_sccb.c \
	video/fsl_ov7725.c \
	video/fsl_video_common.c \
	)	

	
SRC_SYS = $(addprefix $(HAL_DIR)/, \
	system_MIMXRT1052.c \
	)

ifeq ($(CMSIS_MCU),$(filter $(CMSIS_MCU),STM32H743xx))
    SRC_HAL += $(addprefix $(HAL_DIR)/src/stm32$(MCU_SERIES)xx_, hal_fdcan.c)
else
    SRC_HAL += $(addprefix $(HAL_DIR)/src/stm32$(MCU_SERIES)xx_, hal_can.c)
endif

SRC_USBDEV = $(addprefix $(USBDEV_DIR)/,\
	composite.c \
	ring_buffer.c \
	virtual_com.c \
	usb_device_descriptor.c \
	usb_device_msc.c \
	)

ifneq ($(MICROPY_PY_WIZNET5K),0)
WIZNET5K_DIR=drivers/wiznet5k
INC += -I$(TOP)/$(WIZNET5K_DIR)
CFLAGS_MOD += -DMICROPY_PY_WIZNET5K=$(MICROPY_PY_WIZNET5K) -D_WIZCHIP_=$(MICROPY_PY_WIZNET5K)
SRC_MOD += modnwwiznet5k.c
SRC_MOD += $(addprefix $(WIZNET5K_DIR)/,\
	ethernet/w$(MICROPY_PY_WIZNET5K)/w$(MICROPY_PY_WIZNET5K).c \
	ethernet/wizchip_conf.c \
	ethernet/socket.c \
	internet/dns/dns.c \
	)
endif

# for CC3000 module
ifeq ($(MICROPY_PY_CC3K),1)
CC3000_DIR=drivers/cc3000
INC += -I$(TOP)/$(CC3000_DIR)/inc
CFLAGS_MOD += -DMICROPY_PY_CC3K=1
SRC_MOD += modnwcc3k.c
SRC_MOD += $(addprefix $(CC3000_DIR)/src/,\
	cc3000_common.c \
	evnt_handler.c \
	hci.c \
	netapp.c \
	nvmem.c \
	security.c \
	socket.c \
	wlan.c \
	ccspi.c \
	inet_ntop.c \
	inet_pton.c \
	patch.c \
	patch_prog.c \
	)
endif

OBJ =
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_LIBM:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(SRC_UTIL:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_HAL:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_SYS:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_USBDEV:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

all: $(OBJ)

ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

.PHONY: deploy

deploy: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $< to the board"
ifeq ($(USE_PYDFU),1)
	$(Q)$(PYTHON) $(PYDFU) -u $<
else
	$(Q)$(DFU_UTIL) -a 0 -d $(DEVICE) -D $<
endif

# A board should specify TEXT0_ADDR if to use a different location than the
# default for the firmware memory location.  A board can also optionally define
# TEXT1_ADDR to split the firmware into two sections; see below for details.
TEXT0_ADDR ?= 0x08000000

ifeq ($(TEXT1_ADDR),)
# No TEXT1_ADDR given so put all firmware at TEXT0_ADDR location

deploy-stlink: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $(BUILD)/firmware.bin to the board via ST-LINK"
	$(Q)$(STFLASH) write $(BUILD)/firmware.bin $(TEXT0_ADDR)

deploy-openocd: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $(BUILD)/firmware.bin to the board via ST-LINK using OpenOCD"
	$(Q)$(OPENOCD) -f $(OPENOCD_CONFIG) -c "stm_flash $(BUILD)/firmware.bin $(TEXT0_ADDR)"

$(BUILD)/firmware.dfu: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin
	$(Q)$(PYTHON) $(DFU) -b $(TEXT0_ADDR):$(BUILD)/firmware.bin $@

else
# TEXT0_ADDR and TEXT1_ADDR are specified so split firmware between these locations

deploy-stlink: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $(BUILD)/firmware0.bin to the board via ST-LINK"
	$(Q)$(STFLASH) write $(BUILD)/firmware0.bin $(TEXT0_ADDR)
	$(ECHO) "Writing $(BUILD)/firmware1.bin to the board via ST-LINK"
	$(Q)$(STFLASH) --reset write $(BUILD)/firmware1.bin $(TEXT1_ADDR)

deploy-openocd: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $(BUILD)/firmware{0,1}.bin to the board via ST-LINK using OpenOCD"
	$(Q)$(OPENOCD) -f $(OPENOCD_CONFIG) -c "stm_flash $(BUILD)/firmware0.bin $(TEXT0_ADDR) $(BUILD)/firmware1.bin $(TEXT1_ADDR)"

$(BUILD)/firmware.dfu: $(BUILD)/firmware.elf
	$(ECHO) "GEN $@"
	$(Q)$(OBJCOPY) -O binary -j .isr_vector $^ $(BUILD)/firmware0.bin
	$(Q)$(OBJCOPY) -O binary -j .text -j .data $^ $(BUILD)/firmware1.bin
	$(Q)$(PYTHON) $(DFU) -b $(TEXT0_ADDR):$(BUILD)/firmware0.bin -b $(TEXT1_ADDR):$(BUILD)/firmware1.bin $@

endif

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(ECHO) "GEN $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

PLLVALUES = boards/pllvalues.py
MAKE_PINS = boards/make-pins.py
BOARD_PINS = boards/$(BOARD)/pins.csv
PREFIX_FILE = boards/mimxrt105x_prefix.c
PINMAP_FILE = boards/mimxrt105x_pinmap.csv
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

INSERT_USB_IDS = $(TOP)/tools/insert-usb-ids.py
FILE2H = $(TOP)/tools/file2h.py

USB_IDS_FILE = usb_app.h
CDCINF_TEMPLATE = pybcdc.inf_template
GEN_CDCINF_FILE = $(HEADER_BUILD)/pybcdc.inf
GEN_CDCINF_HEADER = $(HEADER_BUILD)/pybcdc_inf.h

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS += $(GEN_CDCINF_HEADER)

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(GEN_PINS_HDR)

# With conditional pins, we may need to regenerate qstrdefs.h when config
# options change.
$(HEADER_BUILD)/qstrdefs.generated.h: boards/$(BOARD)/mpconfigboard.h

# main.c can't be even preprocessed without $(GEN_CDCINF_HEADER)
main.c: $(GEN_CDCINF_HEADER)

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_$(BOARD).c and pins.h
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(BUILD)/%_qstr.h: boards/$(BOARD)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "GEN $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --af-const $(GEN_PINS_AF_CONST) --pinmap $(PINMAP_FILE) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_$(BOARD).o: $(BUILD)/pins_$(BOARD).c
	$(call compile_c)

#GEN_PLLFREQTABLE_HDR = $(HEADER_BUILD)/pllfreqtable.h
GEN_MCUCONST_HDR = $(HEADER_BUILD)/modmcu_const.h
GEN_MCUCONST_QSTR = $(BUILD)/modmcu_qstr.h
GEN_MCUCONST_MPZ = $(HEADER_BUILD)/modmcu_mpz.h
CMSIS_MCU_UPPER = $(shell echo $(CMSIS_MCU) | tr '[:lower:]' '[:upper:]')
CMSIS_MCU_HDR = mcusdk/$(CMSIS_MCU_UPPER).h

#TODO Dave: PLL Table build nessessary?
#modmachine.c: $(GEN_PLLFREQTABLE_HDR)
#$(GEN_PLLFREQTABLE_HDR): $(PLLVALUES) | $(HEADER_BUILD)
#	$(ECHO) "GEN $@"
#	$(Q)$(PYTHON) $(PLLVALUES) -c file:boards/$(BOARD)/stm32$(MCU_SERIES)xx_hal_conf.h > $@

$(BUILD)/modmcu_qstr.h: $(CMSIS_MCU_HDR) make-mcuconst.py | $(HEADER_BUILD)
	$(ECHO) "Create modmcu_qstr.h"
	$(Q)$(PYTHON) make-mcuconst.py -bf --qstr $(GEN_MCUCONST_QSTR) --mpz $(GEN_MCUCONST_MPZ) $(CMSIS_MCU_HDR) > $(GEN_MCUCONST_HDR)
	
$(BUILD)/modmcu.o: $(GEN_MCUCONST_HDR)
# Use a pattern rule here so that make will only call make-stmconst.py once to
# make both modstm_const.h and modstm_qstr.h
$(HEADER_BUILD)/%_const.h $(BUILD)/%_qstr.h: $(CMSIS_MCU_HDR) make-mcuconst.py | $(HEADER_BUILD)
	$(ECHO) "Create mcuconst $@"
	$(Q)$(PYTHON) make-mcuconst.py --qstr $(GEN_MCUCONST_QSTR) --mpz $(GEN_MCUCONST_MPZ) $(CMSIS_MCU_HDR) > $(GEN_MCUCONST_HDR)

$(GEN_CDCINF_HEADER): $(GEN_CDCINF_FILE) $(FILE2H) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_CDCINF_FILE): $(CDCINF_TEMPLATE) $(INSERT_USB_IDS) $(USB_IDS_FILE) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(INSERT_USB_IDS) $(USB_IDS_FILE) $< > $@

include $(TOP)/py/mkrules.mk
