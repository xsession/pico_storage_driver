#ifndef W25Q_H
#define W25Q_H
#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"


/* W25Q SPI pins*/
#define W25Q_SPI_PORT spi0
#define W25Q_PIN_MISO 16
#define W25Q_PIN_SCK  18
#define W25Q_PIN_MOSI 19
#define W25Q_PIN_CS 17
/* ====================== */

typedef struct{
    spi_inst_t *spi;
    uint        cs_pin;
    uint8_t     uuid[8];
    uint32_t    jedec_id;

    uint32_t    blockCount;
    uint32_t    blockSize;
    
    uint32_t    sectorCount;
    uint32_t    sectorSize;

    uint32_t    pageCount;
    uint16_t    pageSize;
    
    uint8_t     statusRegister1;
    uint8_t     statusRegister2;
    uint8_t     statusRegister3;
    uint32_t    capacityKB;
    uint8_t     lock;
    bool        spiInit;
    uint8_t     Stat;
}w25q_data_t;


uint8_t w25q_disk_initialize(spi_inst_t *spi, uint cs_pin, w25q_data_t *w25q);
void w25q_get_manufacter_device_id(uint8_t *mid, w25q_data_t *w25q);
void w25q_get_JEDEC_ID(w25q_data_t *w25q);
void w25q_erase_chip(w25q_data_t *w25q);
void w25q_page_program(uint32_t page_addr, uint16_t offset, uint8_t *buf, uint32_t len, w25q_data_t *w25q);
void w25q_write_sector(uint32_t sect_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q);
void w25q_write_block_64k(uint32_t blk_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q);
void w25q_read_bytes(uint32_t address, uint8_t *buf, uint32_t len, w25q_data_t *w25q);
void w25q_read_page(uint32_t page_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q);
uint8_t w25q_read_sector(uint32_t sect_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q);
void w25q_read_block(uint32_t blk_addr, uint32_t offset, uint8_t *buf, uint32_t len, w25q_data_t *w25q);
//void w25q_read_data(uint32_t address, uint8_t *buf, uint32_t len);
//void w25q_fast_read_data(uint32_t address, uint8_t *buf, uint32_t len);
void w25q_read_status_register_1(w25q_data_t *w25q);
void w25q_read_status_register_2(w25q_data_t *w25q);
void w25q_read_status_register_3(w25q_data_t *w25q);
void w25q_write_status_register_1(w25q_data_t *w25q);
void w25q_write_status_register_2(w25q_data_t *w25q);
void w25q_write_status_register_3(w25q_data_t *w25q);
void w25q_sector_erase(uint32_t sect_addr, w25q_data_t *w25q);
void w25q_block_erase_32k(uint32_t blk_addr,w25q_data_t *w25q);
void w25q_block_erase_64k(uint32_t blk_addr, w25q_data_t *w25q);
void w25q_get_uid(w25q_data_t *w25q);
void w25q_write_enable(w25q_data_t *w25q);
void w25q_write_diable(w25q_data_t *w25q);

#endif