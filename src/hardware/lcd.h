#ifndef LCD_H_
#define LCD_H_

#include "hal_data.h"

#define LCD_RESET     IOPORT_PORT_06_PIN_10
#define LCD_CS        IOPORT_PORT_06_PIN_11
#define LCD_BACKLIGHT IOPORT_PORT_06_PIN_12
#define LCD_CMD       IOPORT_PORT_01_PIN_15

/* ILI9341 command set used by the Renesas sample for SK-S7G2. */
#define ILI9341_SW_RESET         0x01
#define ILI9341_SLEEP_OUT        0x11
#define ILI9341_DISP_ON          0x29
#define ILI9341_COLUMN_ADDR      0x2A
#define ILI9341_PAGE_ADDR        0x2B
#define ILI9341_MAC              0x36
#define ILI9341_PIXEL_FORMAT     0x3A
#define ILI9341_GAMMA            0x26
#define ILI9341_RGB_INTERFACE    0xB0
#define ILI9341_FRM_CTRL1        0xB1
#define ILI9341_DFC              0xB6
#define ILI9341_POWER1           0xC0
#define ILI9341_POWER2           0xC1
#define ILI9341_VCOM1            0xC5
#define ILI9341_VCOM2            0xC7
#define ILI9341_POWERB           0xCF
#define ILI9341_READ_ID4         0xD3
#define ILI9341_PGAMMA           0xE0
#define ILI9341_NGAMMA           0xE1
#define ILI9341_DTCA             0xE8
#define ILI9341_DTCB             0xEA
#define ILI9341_POWER_SEQ        0xED
#define ILI9341_3GAMMA_EN        0xF2
#define ILI9341_INTERFACE        0xF6
#define ILI9341_POWERA           0xCB
#define ILI9341_PRC              0xF7

void lcd_setup(void);

#endif
