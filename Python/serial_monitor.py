import serial
import sys
import struct

ser = serial.Serial('/dev/ttyUSB0', 9600)
text_filter = 'mil'
text_buffer = ''
# while True:
# 	for char in ser.read():
# 		if char == '\n':
# 			text_buffer = ''
# 		else:
# 			if len(text_buffer) < 3:
# 				text_buffer += char
# 		if text_buffer == text_filter:
# 			sys.stdout.write(char)

RASP_SEND_DATA = 0b10000000
RASP_REQUEST_DATA = 0b01000000

RASP_NULL = 0

RASP_LED_ON = 1
RASP_LED_COLOR = 2

RASP_THETA = 5
RASP_RADIUS = 6

RASP_GCODE = 10

RASP_STAYALIVE = 20
RASP_SLEEP = 21

# packet = bytearray()
# packet.append(RASP_SEND_DATA | RASP_LED_ON)
# ser.write(packet)

packet = bytearray()
packet.append(RASP_SEND_DATA | RASP_LED_COLOR)
packet.append(255)
packet.append(128)
packet.append(64)
ser.write(packet)