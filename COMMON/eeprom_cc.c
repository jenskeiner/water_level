/**
 * eeprom_cc.c – EEPROM-Zugriff für ATtiny1616 (NVMCTRL)
 */
#include "eeprom_cc.h"
#include <avr/io.h>

#ifndef EEPROM_START
#define EEPROM_START 0x1400u
#endif

static void ee_wait(void)
{
	while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm) {
		;
	}
}

static uint8_t ee_read_byte(uint16_t addr)
{
	return *(volatile uint8_t *)(EEPROM_START + addr);
}

static void ee_write_byte(uint16_t addr, uint8_t data)
{
	if (ee_read_byte(addr) == data) {
		return; /* schon korrekt – EEPROM schonen */
	}
	ee_wait();

	/* Page-Buffer leeren */
	CCP = CCP_SPM_gc;
	NVMCTRL.CTRLA = NVMCTRL_CMD_PAGEBUFCLR_gc;
	ee_wait();

	*(volatile uint8_t *)(EEPROM_START + addr) = data;

	CCP = CCP_SPM_gc;
	NVMCTRL.CTRLA = NVMCTRL_CMD_PAGEERASEWRITE_gc;
	ee_wait();
}

bool ee_is_calibrated(void)
{
	uint16_t magic = (uint16_t)ee_read_byte(EE_MAGIC_ADDR) |
	                 ((uint16_t)ee_read_byte(EE_MAGIC_ADDR + 1) << 8);
	return (magic == EE_MAGIC);
}

bool ee_load_cc(uint16_t *cc, uint8_t count)
{
	if (!ee_is_calibrated()) {
		return false;
	}
	for (uint8_t i = 0; i < count; i++) {
		uint16_t a = EE_CC_START_ADDR + (uint16_t)(i * 2);
		cc[i] = (uint16_t)ee_read_byte(a) |
		        ((uint16_t)ee_read_byte(a + 1) << 8);
	}
	return true;
}

void ee_save_cc(const uint16_t *cc, uint8_t count)
{
	ee_write_byte(EE_MAGIC_ADDR, (uint8_t)(EE_MAGIC & 0xFF));
	ee_write_byte(EE_MAGIC_ADDR + 1, (uint8_t)(EE_MAGIC >> 8));
	for (uint8_t i = 0; i < count; i++) {
		uint16_t a = EE_CC_START_ADDR + (uint16_t)(i * 2);
		ee_write_byte(a, (uint8_t)(cc[i] & 0xFF));
		ee_write_byte(a + 1, (uint8_t)(cc[i] >> 8));
	}
}

void ee_invalidate(void)
{
	ee_write_byte(EE_MAGIC_ADDR, 0x00);
	ee_write_byte(EE_MAGIC_ADDR + 1, 0x00);
}
