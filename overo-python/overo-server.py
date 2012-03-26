#!/usr/bin/python
import SocketServer
import os
import subprocess
import shlex

GROUND_STATION_IP = "192.168.23.3"
GROUND_STATION_PORT = 9999
VIDEO_PIPELINE = "/usr/bin/gst-launch -v videotestsrc ! video/x-raw-yuv,width=640,height=480 ! TIVidenc1 codecName=h264enc engineName=codecServer ! rtph264pay pt=96 ! udpsink host=192.168.23.4 port=4000"

class MyUDPHandler(SocketServer.BaseRequestHandler):
    
    """
    This class works similar to the TCP handler class, except that
    self.request consists of a pair of data and client socket, and since
    there is no connection the client address must be given explicitly
    when sending data back via sendto().
    """
    #static variable
    video_process = 0

    def handle(self):
        data = self.request[0].strip()
        #socket = self.request[1]
        #print "{} wrote:".format(self.client_address[1])
        print data
	if(data == ',' or data == '-'):
		self.toggle_video(data)
	else:
        	#socket.sendto(data.upper(), self.client_address)
		cmd = "./overo-i2c \""+data+"\""
		print cmd
        	os.system("./overo-i2c \""+data+"\"")
     
    @classmethod
    def toggle_video(self,data):
        if(data == ','):
                if self.video_process == 0:
			args = shlex.split(VIDEO_PIPELINE)
                        self.video_process = subprocess.Popen(args) 
	elif data == '-':
                print self.video_process
		print self.video_process.pid
                subprocess.call(["kill", str(self.video_process.pid)])
                # TODO check if process has really been killed before setting flag
                self.video_process = 0;

if __name__ == "__main__":
    HOST, PORT = GROUND_STATION_IP, GROUND_STATION_PORT
    server = SocketServer.UDPServer((HOST, PORT), MyUDPHandler)
    server.serve_forever()
