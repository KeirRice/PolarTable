#!/usr/bin/env python
"""IC2 coms."""

from smbus2 import SMBusWrapper

# const byte ARDUINO_I2C_ADDRESS = 0x04;
# const byte RASPBERRY_I2C_ADDRESS = 0x14;
# const byte SX1509_I2C_ADDRESS = 0x3E;

ARDUINO_I2C_ADDRESS = 0x04
RASPBERRY_I2C_ADDRESS = 0x14
SX1509_I2C_ADDRESS = 0x3E


class Arduino(object):
    """I2C inteface to the Ardurino project.

    Arduino Register Map:
        0x01 - System State (Sleep/Wake)
        0x02 - Lighting On/Off
        0x03 - Lighting Red
        0x04 - Lighting Green
        0x05 - Lighting Blue
        0x06 - Lighting Blend Time
        0x07 - Motor Theta Steps MSB
        0x08 - Motor Theta Steps LSB
        0x09 - Motor Radius Steps MSB
        0x0A - Motor Radius Steps LSB
    """

    def __init__(self, address):
        """Init."""
        self.address = address

    def sleep(self):
        """Put the whole systems to sleep. This will also sutdown the PI."""
        with SMBusWrapper(1) as bus:
            bus.write_byte_data(self.address, 0x01, 0)
    
    def get_lighting_on(self):
        """Return True if the lighting is on."""
        with SMBusWrapper(1) as bus:
            return bus.read_byte_data(self.address, 0x02) != 0

    def set_lighting(self, turn_on=True):
        """Turn the lighting on or off."""
        with SMBusWrapper(1) as bus:
            bus.write_byte_data(self.address, 0x02, 1 if turn_on else 0)

    def get_lighting_colour(self, h, s, v):
        """Get the lighting colour."""
        with SMBusWrapper(1) as bus:
            return bus.read_i2c_block_data(self.address, 0x03, 3)

    def set_lighting_colour(self, h, s, v):
        """Set the lighting colour."""
        with SMBusWrapper(1) as bus:
            bus.write_i2c_block_data(self.address, 0x03, (h, s, v))

    def get_motor_steps(self):
        """Get the current motor step counts."""
        with SMBusWrapper(1) as bus:
            theta = bus.read_word_data(self.address, 0x07)
            radius = bus.read_word_data(self.address, 0x09)
            return theta, radius
        
    def set_motor_steps(self, theta, radius):
        """Set a new target position for the motors steps."""
        with SMBusWrapper(1) as bus:
            bus.write_word_data(self.address, 0x07, theta)
            bus.write_word_data(self.address, 0x09, radius)

if __name__ == '__main__':
    Arduino(ARDUINO_I2C_ADDRESS)
