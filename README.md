# Firmware for Grove Water Level Sensor 10cm
This repository contains modified sources for the firmware of the two
ATtiny 1616 MCUs on the [Seeed Studio](https://www.seeedstudio.com/)
[Grove Water Level Sensor 10cm](https://www.seeedstudio.com/Grove-Water-Level-Sensor-10CM-p-4443.html).

For each sensor pad, the modified firmware sends the raw sensor value as a
little endian unsigned 16 bit integer over I2C, instead of an unsigned 8 bit
integer that also takes into account the key pressed state of the QTouch
library.

This way, the sensor retains the same range of values even if power-cycled. In
contrast, the original firmware would recalibrate the state of each sensor pad
after a power cycle which would change the values sent over I2C even if the raw
sensor values for each pad hadn't changed. This would lead to incorrect water
level detection since the sensor would indicate that no pad was submerged, even
if if was correctly detecting submerged pads before being power-cycled.

While the changes ensure that the sensor output doesn't change between two power
cycles -- assuming all environmental conditions are still the same -- additional
logic is needed to determine which pads are submerged and which not, to
determine the actual water level.

As a first step, checking the sensor value for each pad against a fixed
threshold is recommended, but further testing is needed to check robustness of
that approach.

Feedback and pull requests welcome.
