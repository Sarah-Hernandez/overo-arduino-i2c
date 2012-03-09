import socket
import Tkinter

LEFT_COMMAND = '40'
RIGHT_COMMAND = '41'
FW_COMMAND = '42'
BACK_COMMAND = '43'

MOTOR_LEFT = '33'
MOTOR_RIGHT = '38'
MOTOR_BOTTOM = '35'
MOTOR_LEFT_RIGHT = '36'
KILL_MOTORS = '37'

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
    buttonBack = Tkinter.Button(self,text="  Up  ",command=self.OnBackClick)
    buttonLeft = Tkinter.Button(self,text="  <-Left  ",command=self.OnLeftClick)
    buttonRight = Tkinter.Button(self,text="Right->  ",command=self.OnRightClick)

    #add labels
    self.labelVariable = Tkinter.StringVar()
    label = Tkinter.Label(self, textvariable = self.labelVariable, anchor='w', fg="black", bg="white")
    self.labelVariable.set("Zepplin Ground Station")
    
    #Add things to the layout manager
    #sticky EW (East West) sticks it to the vertical edges 
    #of the window
    self.entry.grid(column=0,row=0,columnspan=1,sticky='EW')
    #self.w.grid(column=0,row=1,columnspan=2,sticky='EW')
    button.grid(column=1,row=0,columnspan=1,sticky='EW')
    label.grid(column=0,row=1,columnspan=2,sticky='EW')
    buttonFw.grid(column=0,row=2,columnspan=2,sticky='EW')
    buttonBack.grid(column=0,row=4,columnspan=2,sticky='EW')
    buttonLeft.grid(column=0,row=3,columnspan=1,sticky='EW')
    buttonRight.grid(column=1,row=3,columnspan=1,sticky='EW')
    
    #Tell the layout manager to resize columns when the window
    #is resized
    self.grid_columnconfigure(0,weight=1)
    self.grid_columnconfigure(1,weight=2)
    self.resizable(True,False)
    
    #add events
    self.entry.bind("<Return>",self.OnPressEnter)
    self.bind("<Up>",self.OnPressFw)
    self.bind("<Down>",self.OnPressBack)
    self.bind("<Left>",self.OnPressLeft)
    self.bind("<Right>",self.OnPressRight)
    self.bind("<XF86Back>",self.OnMinus)
    self.bind("<XF86Forward>",self.OnPlus)
    
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
    # SOCK_DGRAM is the socket type to use for UDP sockets
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.byteSent = 0
    self.last_cmd = '4'
    self.last_key = 't'
    self.afterId = None

  def OnMinus(self,event):
    slider = self.entry.get()
    slider = slider - 1
    if(slider < 50):
      slider = 50
    #slider = slider%50 + 50
    self.entry.set(slider)
    self.sendCMD(str(slider));
    
  def OnPlus(self,event):
    slider = self.entry.get()
    slider = slider + 1
    if(slider > 100):
      slider = 100
    self.entry.set(slider)
    self.sendCMD(str(slider));
    
  def OnFwClick(self):
    self.sendCMD(MOTOR_LEFT_RIGHT)

  def OnBackClick(self):
    self.sendCMD(MOTOR_BOTTOM)
    
  def OnLeftClick(self):
    self.sendCMD(MOTOR_RIGHT)
    
  def OnRightClick(self):
    self.sendCMD(MOTOR_LEFT)    
    
  def OnButtonClick(self):
    self.PWM = self.entryVar.get()
    if(int(self.PWM) >= 50 and int(self.PWM) <=100):
      self.sendCMD(self.PWM)
    else:
      self.sendCMD(INIT_PWM)
    
  def KillAll(self, event):
    self.afterId = self.after_idle( self.process_release, event )
    #if(event.keysym == self.last_key):
      #print event.keysym+" is still pressed ignore release event"
      #self.last_key = 't'
    #else:
      #self.sendCMD(KILL_MOTORS)
      
  def process_release(self, event):
    print 'key release %s' % event.time
    self.afterId = None
    self.sendCMD(KILL_MOTORS)
   
  def sendCMD(self,cmd1):
    if(cmd1 != self.last_cmd):
      tx1 = int(cmd1)
      print "Sending: {}".format(chr(tx1))
      self.labelVariable.set("Sent : "+str(tx1))
      self.sock.sendto(chr(tx1), (self.HOST, self.PORT))
      self.byteSent = self.byteSent + 1
      print "Sent Total: {} Msgs".format(self.byteSent)
      self.last_cmd = cmd1
    #else:
      #print "last_command : "+self.last_cmd
   
  def OnPressFw(self,event):
    self.OnFwClick()
    self.last_key = event.keysym
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    else:
       print 'key pressed %s' % event.time
    
  def OnPressBack(self,event):
    self.OnBackClick()
    self.last_key = event.keysym
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    else:
       print 'key pressed %s' % event.time
    
  def OnPressLeft(self,event):
    self.OnLeftClick()
    self.last_key = event.keysym
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    else:
       print 'key pressed %s' % event.time

  def OnPressRight(self,event):
    self.OnRightClick()
    self.last_key = event.keysym
    if self.afterId != None:
       self.after_cancel( self.afterId )
       self.afterId = None
    else:
       print 'key pressed %s' % event.time
    
  def OnPressEnter(self,event):
    self.OnButtonClick()
    self.last_key = event.keysym
    
if __name__ == "__main__":
    app = gs_gui(None)
    app.title('zepplin gs')
    app.mainloop()