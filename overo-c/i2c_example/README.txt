   25 Cent I2C Adapter: an Inexpensive Method of Interfacing I2C Devices with
    Personal Computers by Leveraging the Existing Display Data Channel (DDC)

      Copyright 2008 Phillip Burgess    http://www.PaintYourDragon.com/uc/

                 The author assumes no liability for injury or
                damage resulting from the use of this technique

------------------------------------------------------------------------------

This file is part of The 25 Cent I2C Adapter Project.

The 25 Cent I2C Adapter Project is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

The 25 Cent I2C Adapter Project is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
The 25 Cent I2C Adapter Project.  If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------------------------

This project demonstrates the concept of using the Display Data Channel (DDC)
lines present on many graphics cards as frugal method of interfacing with I2C
peripherals.

I2C (Inter-Integrated Circuit) is a two-wire serial bus typically used in
computers for low-level communication between internal components, but is
also seen in robotics and hobbyist electronics for interfacing a variety of
sensors, displays and actuators.  I2C connections are often readily available
on microcontrollers and esoteric embedded systems, but there's little call for
it on mainstream personal computers...unless you happen to be among the
aforementioned robotics and electronics hobbyists, in which case a USB to I2C
adapter device is typically used, often at considerable expense.

DDC - supported by most graphics cards and monitors produced within the
past several years - is a communication channel within a video cable that
allows the computer and display to negotiate mutually compatible resolutions
and permit software control of monitor functions normally accessed with
physical buttons on the display.

DDC is, in fact, simply an implementation of an I2C bus with a few established
rules.  By tapping into this connection between the computer and monitor (or
making use of the DDC lines on an spare unused video port, such as the
external monitor connection on a laptop), one can interface with many I2C
devices at virtually no expense, bypassing the usual need for an adapter
device entirely.

Commercial USB to I2C adapters can cost $100, $250, sometimes even more, and
driver support is spotty for systems outside the popular Windows fold.
Homebrew alternatives exist, but assume a prior investment and knowledge in
microcontroller development.  The method outlined here requires only a
modified video cable...I just bought one at a garage sale for 50 cents, and
that's enough to make TWO such adapters!

Being a quick and dirty hack, there are some limitations to this approach:

1) Most significantly, it does not work in Windows and probably never will.
   Honest, this is not just asinine OS bashing, I've really busted a nut
   trying!  While it's possible in theory, this would entail such a Herculean
   and technically deep development effort that, given the other limitations
   of the scheme and the modest value returned, there's just no sense in it.
   The most suitable Windows software implementation I've found is Nicomsoft's
   WinI2C/DDC developer library, whose documentation details some of the hoops
   they had to jump through.  Given the complexity of the task they've solved
   and the thoroughness of their implementation, the $495 license cost is
   quite a justifiable bargain for big PC graphics hardware OEMs, but the
   hobbyist who's just fiddling around with a few I2C sensors on a Windows
   system will be better served with a USB to I2C adapter (or getting the
   hang of Linux).

2) Even among Mac and Linux systems, the compatible range of hardware is
   limited.  Whether or not application code is allowed access to the DDC
   lines at all is entirely at the whim of whomever wrote the system's video
   driver.  On the Mac, it appears that systems with Intel integrated graphics
   or ATI chipsets support this scheme, while those with NVIDIA graphics are
   out of luck.  I've had few opportunities to test with Linux systems thus
   far, but ATI graphics shows solid support while Intel integrated was a
   no-go (have not tested any NVIDIA systems under Linux yet).

3) The compatible range of I2C devices is limited, and this occasionally
   varies by operating system.  There's inevitably some variation in the
   allowable range of serial timings sent or received by both the host and I2C
   device, and in some combinations these ranges won't overlap.  For example,
   a Nintendo Wii Nunchuk controller will work with Linux but not Mac OS X.  

4) This is limited to being a "single-master" I2C bus - one can poll devices
   for their state, but there's no means for the computer to respond to events
   that originate elsewhere on the bus.  And available power is limited to
   5 volts at a scant 50 milliamps.

5) There's always the possibility of damage by incorrect wiring, so tread
   forth carefully and always mount a scratch monkey.

There are still plenty of fun projects that can be attempted, so with those
caveats out of the way, let's proceed!  All that's required is a modified
video cable, or, if tapping an unused video port, simply the appropriate end
connector (though it's often cheaper to buy the full cable and cut it in
half).  If the port will still be used for a monitor connection, it's
necessary to create a "Y" or "hydra" cable that allows one's I2C device(s)
and the monitor to be attached at the same time (sharing the DDC lines).
When modifying an existing cable, after removing just the outer shielding,
the DDC wires are usually visible at this point, while the lines carrying
video signals are wrapped into further-shielded bundles.  A multimeter or
continuity tester will help correlate wires to pins.  DDC establishes four
wires: +5VDC, ground, serial data and serial clock, the pin numbers of which
will vary depending on the type of video port used.  Refer to pinouts.ru,
Wikipedia or other sources for comprehensive pinout diagrams.

The DDC clock and data lines correspond directly to I2C clock and data.
Pull-up resistors are not required - that's already implemented in the
graphics card - so it's just the cable, your I2C device(s) and a bit of wire.

For a 15-pin VGA cable, the pins of interest are:

   Pin  5:  Ground
   Pin  9:  +5VDC
   Pin 12:  Data
   Pin 15:  Clock

Note that on some VGA cables (perhaps even a majority), pin 9 is not present
(occasionally others as well).  Just examime the male end connector to see if
it's even worth dissecting - the missing pins are quite apparent.  You'll want
to use a cable with the full complement of pins, or work from just a bare
D-sub 15 connector (if scrounging at the local swap meet for cheap cables is
not your style, these connectors can be found at Radio Shack for $1.99).

For a DVI cable:

   Pin  6:  Clock
   Pin  7:  Data
   Pin 14:  +5VDC
   Pin 15:  Ground

For an HDMI Type A cable:

   Pin 15:  Clock
   Pin 16:  Data
   Pin 17:  Ground
   Pin 18:  +5VDC

Connect the corresponding I2C lines to one's device as required, and the other
end of the cable to the desired video port.  Keep in mind that if using a
"hydra" cable with a monitor connected, one's power budget should be trimmed
by a few extra milliamps to ensure enough power for the DDC device within the
display.  There should be enough current to drive a microcontroller and an LED
or possibly two...but if the I2C device(s) to be connected are going to use
any substantial amount of current, it may be wise to power the device(s)
externally and use an I2C buffer circuit where it connects to the cable
(likewise and mandatory if the device operates at a different voltage).  Also,
if using with a display connected, two I2C addresses are reserved for the
monitor and should not be used: 0x37 and 0x50.

While the low-level details of the I2C protocol are taken care of by the i2c.o
library and the OS-specific libraries on which it in turn depends, the message
sequence required by any given device must be implemented within one's own
code and will vary from one device to the next.  Manufacturers' datasheets
will document the specific I2C messages needed, and the amount of code
required is not onerous.  Several short example programs are included:
reading a Nintendo Wii Nunchuk controller accessory (currently Linux only),
reading a Microchip TCN75A temperature sensor, writing to a Microchip 256 Kbit
serial EEPROM, flashing a BlinkM "smart LED," and another that interfaces with
a Mindsensors I2C-SC8 8 channel servo controller.

INSTALLATION:

No special procedure is required for Mac systems, aside from having the Apple
Developer Tools installed in order to compile the code.  If you have a
supported video card and one of the compatible I2C devices attached, simply
compile ('make all') and run (e.g. './temperature').

While the source code for Linux is simpler, the setup procedure unfortunately
is not.  You'll need root access.  Frequently.  This is how I install it under
Ubuntu 7.04 and later:

1) Install the lm-sensors package:
       sudo apt-get install lm-sensors

2) Enable kernel modules for I2C and the specific video driver:
       sudo modprobe i2c-dev
       sudo modprobe radeonfb

   The above is for an ATI Radeon.  You might need a different module specific
   to your system.  Use the command 'sudo modprobe -l' (that's a lowercase L,
   not a one) for a full list of kernel modules, and try to locate one that
   matches your graphics chip vendor.)

   Alternately, to load the I2C modules automatically at boot-time, edit the
   file /etc/modules and add these lines:

       i2c-dev
       radeonfb

   Again, assuming Radeon graphics here; put your actual module name there.

   Once the kernel modules are loaded, the files /dev/i2c-* will then
   correspond to each I2C bus on the system.  Not all of these relate to video
   out though; some may correspond to internal I2C buses in the computer (e.g.
   system health, RAM controller, etc.).  The 'i2cdetect' program (installed
   as part of lm-sensors) will list and briefly describe each I2C interface
   present.  Look for the one that matches the desired video port.  On the
   ThinkPad I used for development, /dev/i2c-2 corresponds to the VGA port.
   If the output of i2cdetect does not mention any video ports (VGA, DVI,
   or HDMI), then it's probable that the driver does not support I2C and this
   hack will not work, or a different video card module may simply need to be
   loaded.

3) Edit the Makefile, commenting out the Mac-specific lines and enabling the
   Linux-related flags.  Then 'make all' to build the library and example
   programs.

4) Access to the /dev/i2c-* devices is normally available only to the root
   user, so there are a few options for invoking the example programs.  The
   most sensible and controlled is to run each with the 'sudo' command, e.g.
   'sudo ./nunchuk'.  If you're on a personal system and can afford to be less
   pedantic about security, other options include logging in and running the
   programs as root, chown/chmod-ing the executables to root and enabling the
   setuid bit, or chmod-ing the /dev/i2c-* files to enable read/write access
   for all users (which would need to be reapplied after each reboot).

USING THE LIBRARY:

The C API is super-basic, with just three functions:

I2Copen()

   Opens the I2C (DDC) bus for subsequent communication.  As some systems may
   support multiple "heads" and thus have multiple DDC interfaces, the code in
   i2c-osx.c or i2c-linux.c may to be adapted to your specific situation.  On
   Macs the code is currently rigged to use the last connection found; on a
   single-head system (e.g. Mac mini), this would be the single video out
   port, while on a potentially two-headed system (e.g. MacBook, late-model
   iMac) this would be the video out port, regardless of whether there's a
   monitor attached.  Multi-headed systems (e.g. Mac Pro) may require some
   tweaks to the code to access a specific graphics card and port.  For Linux,
   the code is rigged to open /dev/i2c-2, which corresponds to the VGA
   connector on my ThinkPad system used during development, but you'll
   probably want to change this for your particular situation.  Review the
   notes above regarding Linux installation.

I2Cmsg(short address, unsigned char *sendBuf, int sendBytes,
  unsigned char *replyBuf, int replyBytes)

   Issues an I2C request and/or reads response over the previously-opened I2C
   bus.  The address of the device is often factory-defined, and the size and
   content of the request data are entirely device-dependent (see the example
   code for several practical cases).  You'll need to work with the datasheet
   provided for your specific device.

   An important point should be made here regarding the first parameter: there
   is some disagreement as to just what constitutes a valid I2C address.  The
   I2C protocol defines data packets in 8-bit chunks, with the first byte of
   an I2C message containing a 7-bit device identifier and one bit indicating
   whether this is a read or write operation.  Most presume the 7-bit value
   to define the address, but in some implementations they'll include the
   read/write bit in referring the address, or - because the R/W flag is the
   least signficant bit - document the device as having two sequential
   addresses (one each for reading and writing).  Linux and OSX disagree here;
   this API follows the 7-bit convention and adjusts the value passed as
   needed for the host operating system.  If a device does not seem to be
   responding at the address given in the datasheet, try dividing the number
   by two, which shifts off the R/W bit to produce a 7-bit address.

I2Cclose()

   Closes the previously-opened I2C bus.

Return values, defined in i2c.h, are currently very limited.  All of the
functions will return a value of zero (or I2C_ERR_NONE) on successful
completion.  The I2Copen() function may return I2C_ERR_OPEN if no available
DDC bus can be found (or, on Linux, if not running as root).  On the Mac,
I2Cmsg() may return various error values defined in
/System/Library/Frameworks/IOKit.framework/Headers/IOReturn.h
(e.g. 0x2c0 or kIOReturnNoDevice).

Neither version of the library is especially comprehensive; there are quite
a few limitations and assumptions made at present (for example, only one I2C
bus can be open at a time).  You might want to look inside and use this as a
starting point for your own code.  The core meat-and-potatoes of opening and
communicating over the bus can be distilled to just a couple dozen essential
lines of code, the rest being error handling, packaging and comments.
