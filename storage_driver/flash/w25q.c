#include "stdio.h"
#include "stdlib.h"
#include "W25Q.h"
#include "storage_driver.h"

uint8_t rxbuf[10];
uint8_t txbuf[10];

/*=================*/

const uint8_t i_uniqueid=0x4b;
const uint8_t i_page_program=0x02;
const uint8_t i_read_data=0x03;
const uint8_t i_fast_read_data=0x0b;
const uint8_t i_write_disable=0x04;
const uint8_t i_read_status_r1=0x05;
const uint8_t i_read_status_r2=0x35;
const uint8_t i_read_status_r3=0x15;
const uint8_t i_write_status_r1=0x01;
const uint8_t i_write_status_r2=0x31;
const uint8_t i_write_status_r3=0x11;
const uint8_t i_sector_erase=0x20;
const uint8_t i_block_erase_32k=0x52;
const uint8_t i_block_erase_64k=0xd8;
const uint8_t i_write_enable=0x06;
const uint8_t i_erase_chip=0xc7;

const uint8_t i_device_id=0x90;
const uint8_t i_JEDEC_ID=0x9f;

void w25q_spi_port_init(w25q_data_t *w25q) {
    gpio_set_dir(w25q->cs_pin, GPIO_OUT);
    gpio_put(w25q->cs_pin, 1);
    gpio_set_function(w25q->cs_pin,   GPIO_FUNC_SIO);
    gpio_set_function(W25Q_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(W25Q_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(W25Q_PIN_MOSI, GPIO_FUNC_SPI);
        
    printf("\nThe actual baudrate(W25Q):%d\n",spi_init(w25q->spi, SPI_BAUDRATE_HIGH));

    w25q->spiInit=true;
}

void w25q_spi_cs_low(w25q_data_t *w25q) {
    gpio_put(w25q->cs_pin,0);
}
void w25q_spi_cs_high(w25q_data_t *w25q){
    gpio_put(w25q->cs_pin,1);
}
void w25q_send_cmd_read(uint8_t cmd, uint32_t address, uint8_t *buf, uint32_t len, bool is_fast, w25q_data_t *w25q) {
    uint8_t addr[4];
    int addr_len=3;
    addr[3] = 0x00;
    if (is_fast) addr_len=4;
    addr[0] = (address & 0x00ff0000) >> 16;
    addr[1] = (address & 0x0000ff00) >> 8;
    addr[2] = (address & 0x000000ff);
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, &cmd, 1);
    spi_write_blocking(w25q->spi, addr, addr_len);
    spi_read_blocking(w25q->spi, 0x00, buf, len);
    w25q_spi_cs_high(w25q);
}

void w25q_send_cmd_write(uint8_t cmd, uint32_t address, uint8_t *buf, uint32_t len, w25q_data_t *w25q) {
    uint8_t addr[3];
    
    addr[0] = (address & 0x00ff0000) >> 16;
    addr[1] = (address & 0x0000ff00) >> 8;
    addr[2] = (address & 0x000000ff);
    w25q_write_enable(w25q);
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, &cmd, 1);
    spi_write_blocking(w25q->spi, addr, 3);
    spi_write_blocking(w25q->spi, buf, len);
    w25q_spi_cs_high(w25q);

}

void w25q_send_cmd_addr(uint8_t cmd, uint32_t address, w25q_data_t *w25q) {
    uint8_t addr[3];
    addr[0] = (address & 0x00ff0000) >> 16;
    addr[1] = (address & 0x0000ff00) >> 8;
    addr[2] = (address & 0x000000ff);
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, &cmd, 1);
    spi_write_blocking(w25q->spi, addr, 3);
    w25q_spi_cs_high(w25q);
}

void w25q_send_cmd(uint8_t cmd, uint8_t *buf, uint32_t len, w25q_data_t *w25q) {
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, &cmd, 1);
    spi_read_blocking(w25q->spi, 0x00, buf, len);
    w25q_spi_cs_high(w25q);
}

void w25q_send_simple_cmd(uint8_t cmd, w25q_data_t *w25q) {
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, &cmd, 1);
    w25q_spi_cs_high(w25q);
}

void w25q_write_enable(w25q_data_t *w25q) {
    w25q_send_simple_cmd(i_write_enable, w25q);
    sleep_ms(1);
}
void w25q_write_disable(w25q_data_t *w25q) {
    w25q_send_simple_cmd(i_write_disable, w25q);
    sleep_ms(1);
}

/*==================*/
uint8_t w25q_disk_initialize(spi_inst_t *spi, uint cs_pin, w25q_data_t *w25q) {
    w25q->spi = spi;
    w25q->cs_pin = cs_pin;

    if (!w25q->spiInit) w25q_spi_port_init(w25q);

    w25q_get_JEDEC_ID(w25q);
    w25q->lock = 1;
	sleep_ms(100);
	switch (w25q->jedec_id & 0x000000FF)
	{
	    case 0x20: // 	w25q512
		    w25q->blockCount = 1024;
		break;
	    case 0x19: // 	w25q256
		    w25q->blockCount = 512;
		break;
	    case 0x18: // 	w25q128
		    w25q->blockCount = 256;
		break;
	    case 0x17: //	w25q64
		    w25q->blockCount = 128;
		break;
	    case 0x16: //	w25q32
		    w25q->blockCount = 64;
		break;
        case 0x15: //	w25q16
            w25q->blockCount = 32;
            break;
        case 0x14: //	w25q80
            w25q->blockCount = 16;
            break;
        case 0x13: //	w25q40
            w25q->blockCount = 8;
        case 0x12: //	w25q20
            w25q->blockCount = 4;
            break;
        case 0x11: //	w25q10
            w25q->blockCount = 2;
            break;
        default:
            w25q->lock = 0;
            return false;
    }
	w25q->pageSize = 256;
	w25q->sectorSize = 0x1000;
	w25q->sectorCount = w25q->blockCount * 16;
	w25q->pageCount = (w25q->sectorCount * w25q->sectorSize) / w25q->pageSize;
	w25q->blockSize = w25q->sectorSize * 16;
	w25q->capacityKB = (w25q->sectorCount * w25q->sectorSize) / 1024;
	w25q_get_uid(w25q);
    w25q_read_status_register_1(w25q);
    w25q_read_status_register_2(w25q);
    w25q_read_status_register_3(w25q);
	w25q->lock = 0;
    w25q->Stat = 0;
	return w25q->Stat;
}


void w25q_read_status_register_1(w25q_data_t *w25q){
    w25q_send_cmd(i_read_status_r1, &w25q->statusRegister1, 1, w25q);
}
void w25q_read_status_register_2(w25q_data_t *w25q){
    w25q_send_cmd(i_read_status_r2, &w25q->statusRegister2, 1, w25q);
}
void w25q_read_status_register_3(w25q_data_t *w25q){
    w25q_send_cmd(i_read_status_r3, &w25q->statusRegister3, 1, w25q);
}

void w25q_write_status_register_1(w25q_data_t *w25q){
    w25q_send_cmd(i_write_status_r1, &w25q->statusRegister1, 1, w25q);
}
void w25q_write_status_register_2(w25q_data_t *w25q){
    w25q_send_cmd(i_write_status_r2, &w25q->statusRegister2, 1, w25q);
}
void w25q_write_status_register_3(w25q_data_t *w25q){
    w25q_send_cmd(i_write_status_r3, &w25q->statusRegister3, 1, w25q);
}

void w25q_wait_for_write_end(w25q_data_t *w25q)
{
	sleep_ms(1);
	w25q_spi_cs_low(w25q);
	spi_write_blocking(w25q->spi, &i_read_status_r1,1);
	do
	{
		spi_read_blocking(w25q->spi, 0x00, &w25q->statusRegister1,1);
		sleep_ms(1);
	} while ((w25q->statusRegister1 & 0x01) == 0x01);
	w25q_spi_cs_high(w25q);
}

void w25q_erase_chip(w25q_data_t *w25q) {
    while (w25q->lock) sleep_ms(1);
    w25q->lock=1;
    w25q_write_enable(w25q);
    w25q_send_simple_cmd(i_erase_chip, w25q);
    w25q_wait_for_write_end(w25q);
    sleep_ms(10);
    w25q->lock=0;
}

void w25q_page_program(uint32_t page_addr, uint16_t offset, uint8_t *buf, uint32_t len, w25q_data_t *w25q) {
    while (w25q->lock) sleep_ms(1);
    w25q->lock=1;
    if (offset + len > w25q->pageSize) {
        len = w25q->pageSize - offset;
    }
    page_addr = (page_addr * w25q->pageSize) + offset;
    w25q_wait_for_write_end(w25q);
    w25q_write_enable(w25q);
    w25q_send_cmd_write(i_page_program, page_addr, buf, len, w25q);
    w25q_wait_for_write_end(w25q);
    sleep_ms(1);
    w25q->lock=0;
}
/*===========================*/
uint32_t w25_page_to_sector_address(uint32_t pageAddress, w25q_data_t *w25q)
{
	return ((pageAddress * w25q->pageSize) / w25q->sectorSize);
}
uint32_t w25q_page_to_block_address(uint32_t pageAddress, w25q_data_t *w25q)
{
	return ((pageAddress * w25q->pageSize) / w25q->blockSize);
}
uint32_t w25q_data_sector_to_block_address(uint32_t sectorAddress, w25q_data_t *w25q)
{
	return ((sectorAddress * w25q->sectorSize) / w25q->blockSize);
}
uint32_t w25q_sector_to_page_address(uint32_t sectorAddress, w25q_data_t *w25q)
{
	return (sectorAddress * w25q->sectorSize) / w25q->pageSize;
}
uint32_t w25q_block_to_page_address(uint32_t blockAddress, w25q_data_t *w25q)
{
	return (blockAddress * w25q->blockSize) / w25q->pageSize;
}
/*============================*/

void w25q_write_sector(uint32_t sect_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q) {
	if (offset >= w25q->sectorSize) return;
    if (offset + len  > w25q->sectorSize) 
		len = w25q->sectorSize - offset;
	uint32_t startPage;
	int32_t bytesToWrite;
	uint32_t localOffset;


    startPage = w25q_sector_to_page_address(sect_addr, w25q) + (offset / w25q->pageSize);
	localOffset = offset % w25q->pageSize;
    bytesToWrite = len;

	do
	{
        w25q_page_program(startPage, localOffset, buf, bytesToWrite, w25q);
		startPage++;
		bytesToWrite -= w25q->pageSize - localOffset;
		buf += w25q->pageSize - localOffset;
		localOffset = 0;
led_blinking_task();

	} while (bytesToWrite > 0);
led_blinking_task_off();

}

void w25q_write_block_64k(uint32_t blk_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q) {
	if ((len > w25q->blockSize) || (len == 0))
		len = w25q->blockSize;
	if (offset >= w25q->blockSize)
		return;
	uint32_t startPage;
	int32_t bytesToWrite;
	uint32_t localOffset;
	if ((offset + len) > w25q->blockSize)
		bytesToWrite = w25q->blockSize - offset;
	else
		bytesToWrite = len;
	startPage = w25q_block_to_page_address(blk_addr, w25q) + (offset / w25q->pageSize);
	localOffset = offset % w25q->pageSize;
	do
	{
		w25q_page_program(startPage, localOffset, buf, len, w25q);
		startPage++;
		bytesToWrite -= w25q->pageSize - localOffset;
		buf += w25q->pageSize - localOffset;
		localOffset = 0;
	} while (bytesToWrite > 0);
   
}

uint8_t w25q_disk_write(
	const uint8_t *buff,	/* Ponter to the data to write */
	uint64_t sector,		/* Start sector number (LBA) */
	uint count, 			/* Number of sectors to write (1..128) */
    w25q_data_t *w25q
) 
{
    uint8_t *tbuf=(uint8_t*)buff;
    while(count > 1)
    {
        w25q_sector_erase(sector, w25q);
        w25q_write_sector(sector, 0, tbuf, w25q->sectorSize, w25q);
        count--;
        tbuf += w25q->sectorSize;
        sector++;
    }
    if (count == 1)
    {
        w25q_sector_erase(sector, w25q);
        w25q_write_sector(sector, 0, tbuf, w25q->sectorSize, w25q);
        count--;
    }
	
	return count? 1: 0;
}

void w25q_read_bytes(uint32_t address, uint8_t *buf, uint32_t len, w25q_data_t *w25q) {
	while (w25q->lock == 1) sleep_ms(1);
	w25q->lock = 1;
    w25q_send_cmd_read(i_fast_read_data, address, buf, len, true, w25q);
	sleep_ms(1);
	w25q->lock = 0;
}

void w25q_read_page(uint32_t page_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q) {
	while (w25q->lock == 1) sleep_ms(1);
	w25q->lock = 1;
    if (offset >= w25q->pageSize) return;
	if ((offset + len) >= w25q->pageSize)
		len = w25q->pageSize - offset;
	page_addr = page_addr * w25q->pageSize + offset;
    w25q_send_cmd_read(i_fast_read_data, page_addr, buf, len, true, w25q);
	
	sleep_ms(1);
	w25q->lock = 0;
}

uint8_t w25q_read_sector(uint32_t sect_addr, uint32_t offset, uint8_t *buf,  uint32_t len, w25q_data_t *w25q) {
	
    if (offset >= w25q->sectorSize) return 0;
    if (offset + len > w25q->sectorSize)
		len = w25q->sectorSize - offset;
	uint32_t startPage;
	int32_t bytesToRead;
	uint32_t localOffset;
    bytesToRead = len;
	
    startPage = w25q_sector_to_page_address(sect_addr, w25q) + (offset / w25q->pageSize);
	localOffset = offset % w25q->pageSize;
	do
	{
		w25q_read_page(startPage, localOffset, buf, bytesToRead, w25q);
    
		startPage++;
		bytesToRead -= w25q->pageSize - localOffset;
		buf += w25q->pageSize - localOffset;
		localOffset = 0;
led_blinking_task();

	} while (bytesToRead > 0);
led_blinking_task_off();

    return 1;

}
void w25q_read_block(uint32_t blk_addr, uint32_t offset, uint8_t *buf, uint32_t len, w25q_data_t *w25q) {
	if (offset+len > w25q->blockSize)
		len = w25q->blockSize-offset;

	uint32_t startPage;
	int32_t bytesToRead;
	uint32_t localOffset;
    bytesToRead = len;

	startPage = w25q_block_to_page_address(blk_addr, w25q) + (offset / w25q->pageSize);
	localOffset = offset % w25q->pageSize;
	do
	{
		w25q_read_page(startPage, localOffset, buf, bytesToRead, w25q);
		startPage++;
		bytesToRead -= w25q->pageSize - localOffset;
		buf += w25q->pageSize - localOffset;
		localOffset = 0;
	} while (bytesToRead > 0);

}


void w25q_sector_erase(uint32_t sect_addr, w25q_data_t *w25q) {
    while(w25q->lock) sleep_ms(1);
    w25q->lock=1;
    sect_addr = sect_addr * w25q->sectorSize;
    w25q_wait_for_write_end(w25q);
    w25q_write_enable(w25q);
    w25q_send_cmd_addr(i_sector_erase, sect_addr, w25q);
    w25q_wait_for_write_end(w25q);
    sleep_ms(1);
    w25q->lock=0;
}
void w25q_block_erase_32k(uint32_t blk_addr, w25q_data_t *w25q) {
    while(w25q->lock) sleep_ms(1);
    w25q->lock=1;
    blk_addr = blk_addr * w25q->sectorSize * 8;
    w25q_wait_for_write_end(w25q);
    w25q_write_enable(w25q);
    w25q_send_cmd_addr(i_block_erase_32k, blk_addr, w25q);
    w25q_wait_for_write_end(w25q);
    sleep_ms(1);
    w25q->lock=0;
}
void w25q_block_erase_64k(uint32_t blk_addr, w25q_data_t *w25q) {
    while(w25q->lock) sleep_ms(1);
    w25q->lock=1;
    blk_addr = blk_addr * w25q->sectorSize * 16;
    w25q_wait_for_write_end(w25q);
    w25q_write_enable(w25q);
    w25q_send_cmd_addr(i_block_erase_64k, blk_addr, w25q);
    w25q_wait_for_write_end(w25q);
    sleep_ms(1);
    w25q->lock=0;
}
void w25q_get_manufacter_device_id(uint8_t *mid, w25q_data_t *w25q){
    assert(w25q->spi);
    w25q_send_cmd_read(i_device_id, 0x000000, mid, 2, false, w25q);
}

void w25q_get_JEDEC_ID(w25q_data_t *w25q) {
    uint8_t temp[3];
    w25q_send_cmd(i_JEDEC_ID, temp, 3, w25q);
    w25q->jedec_id = ((uint32_t)temp[0] << 16) | ((uint32_t)temp[1] << 8) | (uint32_t)temp[2];
}
void w25q_get_uid(w25q_data_t *w25q) {
    assert(w25q->spi);
    txbuf[0]= 0x4b;
    txbuf[1] = 0x00; txbuf[2] = 0x00; txbuf[3] = 0x00;txbuf[4]=0x00;
    w25q_spi_cs_low(w25q);
    spi_write_blocking(w25q->spi, txbuf, 5);
    spi_read_blocking(w25q->spi, 0x00, w25q->uuid, 8);
    w25q_spi_cs_high(w25q);
}