#!/usr/bin/env python
import time
import pigpio

"""
bsc_i2c(i2c_address, data)
This function allows the Pi to act as a slave I2C device. 

The data bytes (if any) are written to the BSC transmit FIFO and the bytes in the BSC receive FIFO are returned. 

Parameters

i2c_address:= the I2C slave address.
       data:= the data bytes to transmit.


The returned value is a tuple of the status, the number of bytes read, and a bytearray containing the read bytes. 

See bsc_xfer for details of the status value. 

If there was an error the status will be less than zero (and will contain the error code). 

Note that an i2c_address of 0 may be used to close the BSC device and reassign the used GPIO (18/19) as inputs. 

This example assumes GPIO 2/3 are connected to GPIO 18/19. 

Example

#!/usr/bin/env python
import time
import pigpio

I2C_ADDR=0x13

def i2c(id, tick):
    global pi

    s, b, d = pi.bsc_i2c(I2C_ADDR)
    if b:
        if d[0] == ord('t'): # 116 send 'HH:MM:SS*'

            print("sent={} FR={} received={} [{}]".
               format(s>>16, s&0xfff,b,d))

            s, b, d = pi.bsc_i2c(I2C_ADDR,
               "{}*".format(time.asctime()[11:19]))

        elif d[0] == ord('d'): # 100 send 'Sun Oct 30*'

            print("sent={} FR={} received={} [{}]".
               format(s>>16, s&0xfff,b,d))

            s, b, d = pi.bsc_i2c(I2C_ADDR,
               "{}*".format(time.asctime()[:10]))

pi = pigpio.pi()

if not pi.connected:
    exit()

# Respond to BSC slave activity

e = pi.event_callback(pigpio.EVENT_BSC, i2c)

pi.bsc_i2c(I2C_ADDR) # Configure BSC as I2C slave

time.sleep(600)

e.cancel()

pi.bsc_i2c(0) # Disable BSC peripheral

pi.stop()


While running the above. 

$ i2cdetect -y 1
    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- 13 -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --

$ pigs i2co 1 0x13 0
0

$ pigs i2cwd 0 116
$ pigs i2crd 0 9 -a
9 10:13:58*

$ pigs i2cwd 0 116
$ pigs i2crd 0 9 -a
9 10:14:29*

$ pigs i2cwd 0 100
$ pigs i2crd 0 11 -a
11 Sun Oct 30*

$ pigs i2cwd 0 100
$ pigs i2crd 0 11 -a
11 Sun Oct 30*

$ pigs i2cwd 0 116
$ pigs i2crd 0 9 -a
9 10:23:16*

$ pigs i2cwd 0 100
$ pigs i2crd 0 11 -a
11 Sun Oct 30*



bsc_xfer(bsc_control, data)
This function provides a low-level interface to the SPI/I2C Slave peripheral. This peripheral allows the Pi to act as a slave device on an I2C or SPI bus. 

I can't get SPI to work properly. I tried with a control word of 0x303 and swapped MISO and MOSI. 

The function sets the BSC mode, writes any data in the transmit buffer to the BSC transmit FIFO, and copies any data in the BSC receive FIFO to the receive buffer. 

Parameters

bsc_control:= see below
       data:= the data bytes to place in the transmit FIFO.


The returned value is a tuple of the status (see below), the number of bytes read, and a bytearray containing the read bytes. If there was an error the status will be less than zero (and will contain the error code). 

Note that the control word sets the BSC mode. The BSC will stay in that mode until a different control word is sent. 

The BSC peripheral uses GPIO 18 (SDA) and 19 (SCL) in I2C mode and GPIO 18 (MOSI), 19 (SCLK), 20 (MISO), and 21 (CE) in SPI mode. You need to swap MISO/MOSI between master and slave. 

When a zero control word is received GPIO 18-21 will be reset to INPUT mode. 

bsc_control consists of the following bits: 

22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 a  a  a  a  a  a  a  -  - IT HC TF IR RE TE BK EC ES PL PH I2 SP EN


Bits 0-13 are copied unchanged to the BSC CR register. See pages 163-165 of the Broadcom peripherals document for full details. 

aaaaaaa defines the I2C slave address (only relevant in I2C mode)
IT  invert transmit status flags
HC  enable host control
TF  enable test FIFO
IR  invert receive status flags
RE  enable receive
TE  enable transmit
BK  abort operation and clear FIFOs
EC  send control register as first I2C byte
ES  send status register as first I2C byte
PL  set SPI polarity high
PH  set SPI phase high
I2  enable I2C mode
SP  enable SPI mode
EN  enable BSC peripheral


The status has the following format: 

20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 S  S  S  S  S  R  R  R  R  R  T  T  T  T  T RB TE RF TF RE TB


Bits 0-15 are copied unchanged from the BSC FR register. See pages 165-166 of the Broadcom peripherals document for full details. 

SSSSS number of bytes successfully copied to transmit FIFO
RRRRR number of bytes in receieve FIFO
TTTTT number of bytes in transmit FIFO
RB  receive busy
TE  transmit FIFO empty
RF  receive FIFO full
TF  transmit FIFO full
RE  receive FIFO empty
TB  transmit busy


Example

(status, count, data) = pi.bsc_xfer(0x330305, "Hello!")
"""

I2C_ADDR = 0x13


def i2c(id, tick):
    """Peripheral mode."""
    global pi

    s, b, d = pi.bsc_i2c(I2C_ADDR)
    if b:
        if d[0] == ord('t'):  # 116 send 'HH:MM:SS*'

            print("sent={} FR={} received={} [{}]".
               format(s >> 16, s & 0xfff, b, d))

            s, b, d = pi.bsc_i2c(I2C_ADDR,
               "{}*".format(time.asctime()[11:19]))

        elif d[0] == ord('d'):  # 100 send 'Sun Oct 30*'

            print("sent={} FR={} received={} [{}]".
               format(s >> 16, s & 0xfff, b, d))

            s, b, d = pi.bsc_i2c(I2C_ADDR,
               "{}*".format(time.asctime()[:10]))

pi = pigpio.pi()

if not pi.connected:
    exit()

# Respond to BSC slave activity

e = pi.event_callback(pigpio.EVENT_BSC, i2c)

pi.bsc_i2c(I2C_ADDR)  # Configure BSC as I2C slave

time.sleep(600)

e.cancel()

pi.bsc_i2c(0)  # Disable BSC peripheral

pi.stop()
