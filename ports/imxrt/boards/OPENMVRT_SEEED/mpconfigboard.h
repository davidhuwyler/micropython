#define MICROPY_HW_BOARD_NAME       "mimxrt1050-evk"
#define MICROPY_HW_MCU_NAME         "i.MX RT105x"
#define MICROPY_HW_UART_REPL    	(repl_uart_id)	// uart ID of REPL uart, must be the same as repl_uart_id in uart.h
#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_HAS_SDCARD       (1)
#define MICROPY_HW_HAS_LCD          (0)
#define MICROPY_HW_ENABLE_RNG       (1)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_CTMR      (0)
#define MICROPY_HW_ENABLE_SERVO     (0)
#define MICROPY_HW_ENABLE_DAC       (0)
#define MICROPY_HW_ENABLE_CAN       (0)
#define MICROPY_MW_ENABLE_SWIM		(0)


//SeeedStudio RGB LED
#define MICROPY_HW_LED1             (pin_AD_B0_09) // red   GPIO_AD_B0_09
#define MICROPY_HW_LED2             (pin_AD_B0_10) // green GPIO_AD_B0_10
#define MICROPY_HW_LED3             (pin_AD_B0_11) // blue  GPIO_AD_B0_11


// XTAL is 12MHz

