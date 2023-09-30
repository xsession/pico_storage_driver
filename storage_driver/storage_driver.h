#ifndef _STORAGE_DRIVER_H_
#define _STORAGE_DRIVER_H_
/* used by my project */

#define SPI_BAUDRATE_LOW (1000*1000)
#define SPI_BAUDRATE_HIGH (40*1000*1000)

enum {
    SDMMC_LUN=0,
    W25Q_LUN=1,
} DISK_LUN;

/* =================================  */
#define  LED_BLINKING_PIN     25
void led_blinking_task(void);
void led_blinking_task_off(void);
/* =================================  */

void storage_driver_init(void);

#endif