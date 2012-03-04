import SocketServer
#import os
import fcntl;
import sys;

GS_IP = "192.168.23.4"
GS_PORT = 9999

class I2CRelay(SocketServer.BaseRequestHandler):
    
    """
    This class works similar to the TCP handler class, except that
    self.request consists of a pair of data and client socket, and since
    there is no connection the client address must be given explicitly
    when sending data back via sendto().
    """
    
    def __init__(self):
      self.IOCTL_I2C_SLAVE = 0x0703
      self.ARDUINO_SLAVE_ADDRESS = 0x10
      self.I2C_DEVICE_NAME = '/dev/i2c-3'
      self.fh = open(I2C_DEVICE_NAME, 'r+', 1)

    def __del__ (self):
      fh.close()
    
    def handle(self):
      #recieve udp packet
      data = self.request[0].strip()
      #send data over i2c bus
      fcntl.ioctl(fh, IOCTL_I2C_SLAVE, ARDUINO_SLAVE_ADDRESS)
      if len(sys.argv) > 1:
	buff = data
	if len(buff) > 31:
	  buff = buff[:31]
	else:
	  buff = 'heartbeat'
	#send
	fh.write(buff)
	##debug send
	#bufflen = len(buff)
	#print 'Sending:', buff

	##debug echo
	buff = fh.read(bufflen)
	if len(buff) > 0:
	  print 'echo:', buff 

if __name__ == "__main__":
    HOST, PORT = GS_IP, GS_PORT
    server = SocketServer.UDPServer((HOST, PORT), I2CRelay)
    server.serve_forever()
