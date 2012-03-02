import socket
import sys
import getch

HOST, PORT = "192.168.23.2", 9999
data = " ".join(sys.argv[1:])

# SOCK_DGRAM is the socket type to use for UDP sockets
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# As you can see, there is no connect() call; UDP has no connections.
# Instead, data is directly sent to the recipient via sendto().

x = 'a'
byteSent = 0
while x != 'q' :
  #x = raw_input("command : ")
  print "command : "
  x = getch.getch()
  sock.sendto(x + "\n", (HOST, PORT))
  byteSent = byteSent + 1
  
print "Sent: {} Bytes".format(byteSent)
