import socket
import Tkinter
import platform
import os
import subprocess
import shlex
import time
from threading import Timer

MOTOR_LEFT = '33'
MOTOR_RIGHT = '38'
MOTOR_BOTTOM = '35'
MOTOR_LEFT_RIGHT = '36'
KILL_MOTORS = '37'

LIFT_OFF = '38'
MANUAL_MODE = '39'
LAND = '46'

LEFT_COMMAND = '40'
RIGHT_COMMAND = '41'
FW_COMMAND = '42'
BACK_COMMAND = '43'

VIDEOSTART = '44'
VIDEOSTOP = '45'
VIDEOENABLED = False
VIDEOPIPELINE = "/usr/bin/gst-launch-0.10 -v udpsrc port=4000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! rtph264depay ! ffdec_h264 ! xvimagesink sync=false"

INIT_PWM = '50'

AUTOPILOT_IP = "192.168.23.3"
AUTOPILOT_PORT = 9999

class gs_gui(Tkinter.Tk):
  
    #Track parent
    #Gui's are a hierarchy of thimgs
    #ex: Window-->Tab-->Pane-->Button
    #    (parent) ---> children

  def __init__(self,parent):
    Tkinter.Tk.__init__(self,parent)
    #Good practice to mainatain a reference to your parent
    self.parent = parent
    #Create gui elements
    self.inititalize()
 
  def inititalize(self):
    #Instanstiate a layout manager
    self.grid()
    
    #create a text entry widget
    #we keep a refference to the txt box in order to
    #read / manipulate it later
    #variable to store txt entered
    #self.entryVar = Tkinter.StringVar()
    #self.entryVar.set(5
    #self.entry = Tkinter.Entry(self,textvariable=self.entryVar)
    #self.entryVar.set("Enter Custom Command")
    
    #create a scale widget
    self.entryVar = Tkinter.StringVar()
    self.entryVar.set(50)
    self.entry = Tkinter.Scale(self, from_=50, to=100, orient="horizontal",variable=self.entryVar)
    
    #create a buttons
    #we do not keep a refference to the button since
    #it will not be changed
    button = Tkinter.Button(self,text="Send",command=self.OnButtonClick)
    buttonFw = Tkinter.Button(self,text=" Forward ",command=self.OnFwClick)
    buttonBack = Tkinter.Button(self,text="   Up    ",command=self.OnBackClick)
    buttonLeft = Tkinter.Button(self,text=" <-Left  ",command=self.OnLeftClick)
    buttonRight = Tkinter.Button(self,text="Right->  ",command=self.OnRightClick)
    buttonLiftOff = Tkinter.Button(self,text=" Hover ",command=self.OnLiftOffClick)
    buttonManual = Tkinter.Button(self,text="  Manual   ",command=self.OnManualClick)
    buttonLand = Tkinter.Button(self,text="  Land  ",command=self.OnLandClick)    
    buttonKill = Tkinter.Button(self,text=" KillAll ",command=self.OnKillAllClick)
    buttonVideo = Tkinter.Button(self,text=" Video ",command=self.OnVideoClick)
    
    #add labels
    self.labelVariable = Tkinter.StringVar()
    label = Tkinter.Label(self, textvariable = self.labelVariable, anchor='w', fg="black", bg="white")
    self.labelVariable.set("Zepplin Ground Station")
    self.modeVariable = Tkinter.StringVar()
    mode = Tkinter.Label(self, textvariable = self.labelVariable, anchor='w', fg="black", bg="white")
    self.modeVariable.set("Zepplin Ground Station")
    
    #Add things to the layout manager
    #sticky EW (East West) sticks it to the vertical edges 
    #of the window
    self.entry.grid(column=0,row=0,columnspan=1,sticky='EW')
    #self.w.grid(column=0,row=1,columnspan=2,sticky='EW')
    button.grid(column=1,row=0,columnspan=1,sticky='EW')
    label.grid(column=0,row=1,columnspan=2,sticky='EW')
    buttonFw.grid(column=0,row=2,columnspan=2,sticky='EW')
    buttonLeft.grid(column=0,row=3,columnspan=1,sticky='EW')
    buttonRight.grid(column=1,row=3,columnspan=1,sticky='EW')
    buttonBack.grid(column=0,row=4,columnspan=2,sticky='EW')
    buttonLiftOff.grid(column=0,row=5,columnspan=2,sticky='EW')
    buttonManual.grid(column=0,row=6,columnspan=2,sticky='EW')
    buttonLand.grid(column=0,row=7,columnspan=2,sticky='EW')
    buttonKill.grid(column=0,row=8,columnspan=2,sticky='EW')
    buttonVideo.grid(column=0,row=9,columnspan=2,sticky='EW')
    
    
    #Tell the layout manager to resize columns when the window
    #is resized
    self.grid_columnconfigure(0,weight=1)
    self.grid_columnconfigure(1,weight=1)
    self.resizable(True,False)
    
    #add events
    self.entry.bind("<Return>",self.OnPressEnter)
    self.bind("<Up>",self.OnPressFw)
    self.bind("<Down>",self.OnPressBack)
    self.bind("<Left>",self.OnPressLeft)
    self.bind("<Right>",self.OnPressRight)
    self.bind("<space>",self.KillAll)
    
    try:
      self.bind("<XF86Back>",self.OnMinus)
      self.bind("<XF86Forward>",self.OnPlus)
      self.bind("<XF86Launch1>",self.LiftOff)
      self.bind("<XF86Launch2>",self.Land)
    except Exception:
      print "Your platform does not support XF86 key bindings"
      pass
    
    self.bind("<KeyRelease-Up>",self.KillAll)
    self.bind("<KeyRelease-Down>",self.KillAll)
    self.bind("<KeyRelease-Left>",self.KillAll)
    self.bind("<KeyRelease-Right>",self.KillAll)
    
    #init logic
    self.initClient()

  def initClient(self): 
    # Open Port
    self.HOST, self.PORT = AUTOPILOT_IP, AUTOPILOT_PORT
    self.PWM = INIT_PWM
    self.VIDEOENABLED = VIDEOENABLED
    self.video_process = 0
    # SOCK_DGRAM is the socket type to use for UDP sockets
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.byteSent = 0
    self.last_cmd = 'BRRR'
    self.afterId = None
    #reset controller
    self.sendCMD(KILL_MOTORS)
    self.sendCMD(INIT_PWM)

  def OnMinus(self,event):
    slider = self.entry.get()
    slider = slider - 1
    if(slider < 50):
      slider = 50
    #slider = slider%50 + 50
    self.entry.set(slider)
    self.sendCMD(str(slider));
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif event != None:
       print '- pressed %s' % event.time
    
  def OnPlus(self,event):
    slider = self.entry.get()
    slider = slider + 1
    if(slider > 100):
      slider = 100
    self.entry.set(slider)
    self.sendCMD(str(slider));
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif event != None:
       print '+ pressed %s' % event.time    
    
  def LiftOff(self,event):
    self.modeVariable.set("Hover")
    self.sendCMD(LIFT_OFF)
    
  def Manual(self,event):
    self.modeVariable.set("Manual")
    self.sendCMD(MANUAL_MODE)
    
  def Land(self,event):
    self.modeVariable.set("Land")
    self.sendCMD(LAND)
  
  def OnLiftOffClick(self):
    #self.sendCMD(MOTOR_LEFT_RIGHT)
    self.LiftOff(self)
    
  def OnManualClick(self):
    #self.sendCMD(MOTOR_LEFT_RIGHT)
    self.Manual(self)
    
  def OnLandClick(self):
    #self.sendCMD(MOTOR_LEFT_RIGHT)
    self.Land(self)
    
  def OnKillAllClick(self):
    #self.sendCMD(MOTOR_LEFT_RIGHT)
    self.KillAll(self)
    
  def OnVideoClick(self):
    self.ToggleVideo(self);
  
  def OnFwClick(self):
    #self.sendCMD(MOTOR_LEFT_RIGHT)
    self.OnPressFw(self)

  def OnBackClick(self):
    #self.sendCMD(MOTOR_BOTTOM)
    self.OnPressBack(self)
    
  def OnLeftClick(self):
    #self.sendCMD(MOTOR_RIGHT)
    self.OnPressLeft(self)
    
  def OnRightClick(self):
    #self.sendCMD(MOTOR_LEFT)    
    self.OnPressRight(self)
    
  def OnButtonClick(self):
    self.PWM = self.entryVar.get()
    if(int(self.PWM) >= 50 and int(self.PWM) <=100):
      self.sendCMD(self.PWM)
    else:
      self.sendCMD(INIT_PWM)
    
  def KillAll(self, event):
    self.afterId = self.after_idle( self.process_release, event)
      
  def process_release(self, event):
    if hasattr(event,'time'):
      print 'key release %s' % event.time
    else:
      print 'click event'
    self.afterId = None
    self.sendCMD(KILL_MOTORS)
    
  def start_video(self):
    self.sendCMD(VIDEOSTART)
    
  def ToggleVideo(self,event):
    if self.VIDEOENABLED:
      self.sendCMD(VIDEOSTOP)
      if self.video_process != 0:
	subprocess.call(["/bin/kill",str(self.video_process.pid)])
      #TODO check if process has really been killed before setting flag
      self.video_process = 0
      self.VIDEOENABLED = False
    else:
      args = shlex.split(VIDEOPIPELINE)
      print args
      self.video_process = subprocess.Popen(args)
      print self.video_process.pid
      #time.sleep(5)
      t = Timer(5.0,self.start_video)
      t.start()
      self.VIDEOENABLED = True
   
  def sendCMD(self,cmd1):
    if(cmd1 != self.last_cmd):
      tx1 = int(cmd1)
      print "Sending: {}".format(chr(tx1))
      self.labelVariable.set("Sent : "+str(tx1))
      self.sock.sendto(chr(tx1), (self.HOST, self.PORT))
      self.byteSent = self.byteSent + 1
      print "Sent Total: {} Msgs".format(self.byteSent)
      self.last_cmd = cmd1
    else:
      print "last_command : "+self.last_cmd
   
  def OnPressFw(self,event):
    self.sendCMD(MOTOR_LEFT_RIGHT)
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif hasattr(event,'time'):
       print 'FW pressed %s' % event.time
    
  def OnPressBack(self,event):
    self.sendCMD(MOTOR_BOTTOM)
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif hasattr(event,'time'):
       print 'Up pressed %s' % event.time
    
  def OnPressLeft(self,event):
    self.sendCMD(MOTOR_LEFT)
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif hasattr(event,'time'):
       print 'Left pressed %s' % event.time

  def OnPressRight(self,event):
    self.sendCMD(MOTOR_RIGHT)
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    elif hasattr(event,'time'):
       print 'Right pressed %s' % event.time
    
  def OnPressEnter(self,event):
    self.OnButtonClick()
    
if __name__ == "__main__":
    app = gs_gui(None)
    app.title('zepplin gs')
    app.mainloop()