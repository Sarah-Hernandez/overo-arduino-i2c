import socket
import sys
import getch
import Tkinter

class gs_gui(Tkinter.Tk):
  
  def __init__(self,parent):
    Tkinter.Tk.__init__(self,parent)
    #Track parent
    #Gui's are a hierarchy of thimgs
    #ex: Window-->Tab-->Pane-->Button
    #    (parent) ---> children
    #Good practice to mainatain a reference to your parent
    self.parent = parent
    
    #Init the GUI
    #create gui elements
    self.inititalize()
 
  def inititalize(self):
    #instanstiate a layout manager
    self.grid()
    
    #variable to store txt entered
    self.entryVar = Tkinter.StringVar()
    
    #create a text entry widget
    #we keep a refference to the txt box in order to
    #read / manipulate it later
    self.entry = Tkinter.Entry(self,textvariable=self.entryVar)
    
    #add to the layout manager
    #sticky EW (East West) sticks it to the vertical edges 
    #of the window
    self.entry.grid(column=0,row=0,sticky='EW')
    
    #add event
    self.entry.bind("<Return>",self.OnPressEnter)
    
    #set textvariable
    self.entryVar.set("CMD")
    
    #create a button
    #we do not keep a refference to the button since
    #it will not be changed
    button = Tkinter.Button(self,text="Send",command=self.OnButtonClick)
    
    #add it to the layout manager
    button.grid(column=1,row=0)
    
    #add labels
    self.labelVariable = Tkinter.StringVar()
    label = Tkinter.Label(self, textvariable = self.labelVariable, anchor='w', fg="black", bg="white")
    label.grid(column=0,row=1,columnspan=2,sticky='EW')
    self.labelVariable.set("Ground Station")
    
    #Tell the layout manager to resize columns when the window
    #is resized
    self.grid_columnconfigure(0,weight=1)
    self.resizable(True,False)

    # Open Port
    self.HOST, self.PORT = "192.168.23.3", 9999
    # SOCK_DGRAM is the socket type to use for UDP sockets
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.byteSent = 0
    
  def OnButtonClick(self):
    self.labelVariable.set("sending : "+self.entryVar.get())
    print self.entryVar.get()
    self.sock.sendto(self.entryVar.get() + "\n", (self.HOST, self.PORT))
    self.byteSent = self.byteSent + 1
    print "Sent: {} Msgs".format(self.byteSent)
    
  def OnPressEnter(self,event):
    self.OnButtonClick()
    
    
if __name__ == "__main__":
    app = gs_gui(None)
    app.title('zepplin gs')
    app.mainloop()
