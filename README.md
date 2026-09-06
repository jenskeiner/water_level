# Firmware for Grove Water Level Sensor 10cm

Modified firmware for the two ATtiny1616 MCUs on the
[Seeed Studio Grove Water Level Sensor 10cm](https://www.seeedstudio.com/Grove-Water-Level-Sensor-10CM-p-4443.html).

Based on the work of [jenskeiner/water_level](https://github.com/jenskeiner/water_level)
and the original sources from [Pillar1989/water_level](https://github.com/Pillar1989/water_level).

## Problem

The original firmware uses the QTouch library’s key-pressed logic. After a
power cycle the library recalibrates and assumes every pad is dry. If the
sensor is still submerged, the reported level becomes wrong.

jenskeiner’s 0.9.0 firmware already sends the raw 16-bit sensor values instead
of the key state. That keeps the *range* of values stable, but the QTouch
acquisition still recalibrates the compensation capacitors (CC) on every boot.
Therefore the absolute raw values still shift after a power cycle.

## Solution in this fork

1. **Raw 16-bit values** are sent over I2C (little-endian), same as 0.9.0.
2. **Compensation capacitor (CC) values** are stored in the ATtiny’s EEPROM.
3. On boot the firmware loads the stored CC values and **skips** the automatic
   calibration.
4. Calibration is only performed when explicitly requested via I2C.
5. A status command reports whether valid calibration data is present.

After a one-time calibration with a **dry** sensor, the raw values stay
consistent across power cycles (under the same environmental conditions).

## I2C protocol

| Direction | Address | Data | Meaning |
|-----------|---------|------|---------|
| Write | 0x77 / 0x78 | `0x01` | Calibrate + save CC to EEPROM (sensor must be **dry**) |
| Write | 0x77 / 0x78 | `0x02` | Request status (next read returns 2 status bytes) |
| Write | 0x77 / 0x78 | `0x03` | Invalidate stored calibration |
| Read  | 0x77 | 16 bytes | 8 × uint16 LE raw values (lower pads) |
| Read  | 0x78 | 24 bytes | 12 × uint16 LE raw values (upper pads) |

**Status flags** (first byte after command `0x02`):

- `bit0` = `CALIBRATED` – valid CC data in EEPROM  
- `bit1` = `BUSY` – calibration currently running  

## Flashing

You need:

- USB-to-serial adapter
- 4.7 kΩ resistor
- Pin headers soldered to the UPDI pads on the back of the sensor
- [pyupdi](https://github.com/mraardvark/pyupdi) or Microchip Studio + UPDI programmer

Flash **both** MCUs (`u1` = upper 12 pads @ 0x78, `u2` = lower 8 pads @ 0x77).

## First-time calibration

1. Make sure the sensor is completely dry.
2. Power the sensor.
3. From the host (e.g. MicroPython) send command `0x01` to both addresses.
4. Wait ~1 s.
5. Check status (`0x02`) – both chips should report `CALIBRATED`.

After that the calibration survives power cycles.

## Determining water level

Apply a fixed threshold to each pad’s raw value (typical dry ≈ 500–530,
wet ≈ 900–1100; a good starting threshold is 700). Count how many pads from
the bottom exceed the threshold and multiply by 5 to get millimetres.

Example MicroPython library and demo are provided in the `micropython/` folder
of this repository (or in the release assets).

## Building

1. Open the two Atmel Studio / Microchip Studio solutions:
   - `WaterLevel_u1_driver/.../WaterLevel_u1_driver.atsln`
   - `WaterLevel_u2_driver/.../WaterLevel_u2_driver.atsln`
2. Add `COMMON/eeprom_cc.c` to both projects.
3. Apply the changes described in `PATCH_touch.c.md` (or already present in
   this branch) to `qtouch/touch.c` and `touch.h`.
4. Build and flash.

## Credits

- Original sensor firmware: Seeed Studio / Pillar1989
- Raw-value modification: [jenskeiner](https://github.com/jenskeiner/water_level)
- EEPROM calibration & I2C commands: this fork

Feedback and pull requests welcome.