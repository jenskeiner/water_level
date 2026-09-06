/**
 * eeprom_cc.h – Persistente Speicherung der QTouch Compensation Capacitors (CC)
 * für Grove Water Level Sensor (ATtiny1616).
 *
 * EEPROM-Layout:
 *   0..1  Magic 0xC5C5 (LE)
 *   2..   node_comp_caps[0..CHAN_CNT-1] als uint16_t LE
 */
#ifndef EEPROM_CC_H
#define EEPROM_CC_H

#include <stdint.h>
#include <stdbool.h>

#define EE_MAGIC          0xC5C5u
#define EE_MAGIC_ADDR     0u
#define EE_CC_START_ADDR  2u

/**
 * Lädt CC-Werte aus dem EEPROM.
 * @return true wenn Magic gültig und Werte geladen wurden
 */
bool ee_load_cc(uint16_t *cc, uint8_t count);

/**
 * Speichert CC-Werte + Magic ins EEPROM.
 */
void ee_save_cc(const uint16_t *cc, uint8_t count);

/**
 * Prüft nur, ob gültige Kalibrierdaten vorhanden sind.
 */
bool ee_is_calibrated(void);

/**
 * Löscht die Magic (Kalibrierung ungültig machen).
 */
void ee_invalidate(void);

#endif /* EEPROM_CC_H */
