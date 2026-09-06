/**
 * main.c – WaterLevel u2 (8 untere Pads, I2C 0x77)
 *
 * Custom-Firmware:
 *  - Sendet immer 16-Bit Rohwerte (LE) über I2C
 *  - Manuelle Kalibrierung per I2C-Befehl 0x01 → CC in EEPROM speichern
 *  - Beim Boot: gespeicherte CC laden (keine Auto-Kalibrierung)
 *  - Status per I2C-Befehl 0x02 abfragbar
 */
#include <atmel_start.h>
#include <atomic.h>
#include "touch.h"
#include "eeprom_cc.h"

#define CHAN_CNT            8

/* I2C-Befehle (Write-Register) */
#define CMD_CALIBRATE_SAVE  0x01
#define CMD_GET_STATUS      0x02
#define CMD_INVALIDATE      0x03

/* Status-Bits */
#define STATUS_CALIBRATED   0x01
#define STATUS_BUSY         0x02

volatile uint8_t slave_address;
volatile uint8_t register_address;
volatile uint8_t num_addresses;
volatile uint8_t num_reads;
volatile uint8_t num_writes;
volatile uint8_t num_stops;

extern volatile uint8_t measurement_done_touch;

volatile uint16_t raw_value[CHAN_CNT];
volatile uint8_t  read_flag     = 0;
volatile uint8_t  byte_idx      = 0;
volatile uint8_t  pending_cmd   = 0;
volatile uint8_t  status_mode   = 0;
volatile uint8_t  status_byte   = 0;
volatile uint8_t  calibrating   = 0;

extern void touch_calibrate_all(void);
extern void touch_save_current_cc(void);
extern uint16_t get_sensor_node_signal(uint16_t node);

static void update_raw_values(void)
{
	for (uint8_t i = 0; i < CHAN_CNT; i++) {
		raw_value[i] = get_sensor_node_signal(i);
	}
}

void address_handler(void)
{
	slave_address = I2C_0_read();
	I2C_0_send_ack();
	num_addresses++;
	read_flag  = 0;
	byte_idx   = 0;
}

void read_handler(void)
{
	if (status_mode) {
		if (byte_idx == 0) {
			I2C_0_write(status_byte);
		} else {
			I2C_0_write(0x00);
			status_mode = 0;
		}
		byte_idx = (byte_idx + 1) % 2;
	} else {
		if (byte_idx == 0) {
			I2C_0_write((uint8_t)(raw_value[read_flag] & 0x00FF));
		} else {
			I2C_0_write((uint8_t)((raw_value[read_flag] & 0xFF00) >> 8));
		}
		byte_idx = (byte_idx + 1) % 2;
		if (byte_idx == 0) {
			read_flag = (read_flag + 1) % CHAN_CNT;
		}
	}
	num_reads++;
}

void write_handler(void)
{
	register_address = I2C_0_read();
	I2C_0_send_ack();
	num_writes++;
	pending_cmd = register_address;
}

void stop_handler(void)
{
	num_stops++;
}

void error_handler(void)
{
	while (1) {
		;
	}
}

int main(void)
{
	atmel_start_init();
	ENABLE_INTERRUPTS();

	I2C_0_set_read_callback(read_handler);
	I2C_0_set_write_callback(write_handler);
	I2C_0_set_address_callback(address_handler);
	I2C_0_set_stop_callback(stop_handler);
	I2C_0_set_collision_callback(error_handler);
	I2C_0_set_bus_error_callback(error_handler);
	I2C_0_open();

	cpu_irq_enable();

	while (1) {
		touch_process();

		if (measurement_done_touch == 1) {
			update_raw_values();
		}

		if (pending_cmd != 0) {
			uint8_t cmd = pending_cmd;
			pending_cmd = 0;

			if (cmd == CMD_CALIBRATE_SAVE) {
				calibrating = 1;
				touch_calibrate_all();
				for (uint8_t n = 0; n < 25; n++) {
					touch_process();
					for (volatile uint32_t d = 0; d < 40000UL; d++) {
						;
					}
				}
				touch_save_current_cc();
				calibrating = 0;
			} else if (cmd == CMD_GET_STATUS) {
				status_byte = 0;
				if (ee_is_calibrated()) {
					status_byte |= STATUS_CALIBRATED;
				}
				if (calibrating) {
					status_byte |= STATUS_BUSY;
				}
				status_mode = 1;
			} else if (cmd == CMD_INVALIDATE) {
				ee_invalidate();
			}
		}

		for (volatile int i = 0; i < 20000; i++) {
			;
		}
	}
}
