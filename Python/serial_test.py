# ser = serial.Serial('/dev/ttyACM0', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=3)
import telnetlib
import time
from cobs import cobsr as cobs

ser = telnetlib.Telnet('192.168.1.7', 23)

zeroByte = b'\x00' # COBS 1-byte delimiter is hex zero as a (binary) bytes character

def read():
	string = ser.read_until( zeroByte ) # read until the COBS packet ending delimiter is found
	n=len(string)
	if n>0:
		decodeStr = string[:-1] # take everything except the trailing zero byte, b'\x00'
		res = cobs.decode( decodeStr ) # recover binary data encoded on Arduino
		n_binary = len(res)
		
		if (n_binary==expectedBytes):
			return res

def write(string):
	res = cobs.encode(string)
	ser.write(res)
	print res


# num = struct.unpack('fff',res) # voila - floats in python from the Arduino

if __name__ == '__main__':
	i = 0
	while True:
		# struct.unpack('fff',res)
		write('python {}\n'.format(i))
		i += 1
		time.sleep(3)