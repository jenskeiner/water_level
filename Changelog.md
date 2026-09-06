# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.9.0] - 2020-09-22
### Changed
- For each sensor pad, the raw sensor value is now sent over I2C as a little
  endian unsigned 16 bit integer.
  
## [1.0.0] - 2026-09-06

### Added
* Persistent storage of QTouch compensation capacitor (CC) values in EEPROM.
* I2C command 0x01 – trigger calibration and save CC values (sensor must be dry).
* I2C command 0x02 – read calibration status (CALIBRATED / BUSY).
* I2C command 0x03 – invalidate stored calibration.
* On boot the firmware loads stored CC values and skips automatic recalibration.

### Changed
* `touch_sensors_config()` no longer always calls `qtm_calibrate_sensor_node()`.
* `main.c` (u1 + u2) handles the new I2C commands in the main loop.

### Notes
* A one-time dry calibration is required after flashing.
* Raw 16-bit little-endian values are still sent on every read (compatible with 0.9.0 host code).