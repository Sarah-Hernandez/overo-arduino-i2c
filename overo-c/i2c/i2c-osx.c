/****************************************************************************
 File        : i2c-osx.c
 Description : Mac OS X-specific code for communicating with the DDC (I2C)
               bus on the video out port.  The idea is that alternate
               versions of this library could be implemented for other
               systems (e.g. Linux, Win32) using a common API, so any code
               linked with it is then otherwise portable.
 History     : 4/1/2008  P. Burgess  Initial implementation
 License     : Copyright 2008 Phillip Burgess

               This file is part of The 25 Cent I2C Adapter Project.

               The 25 Cent I2C Adapter Project is free software: you can
               redistribute it and/or modify it under the terms of the GNU
               General Public License as published by the Free Software
               Foundation, either version 3 of the License, or (at your
               option) any later version.

               The 25 Cent I2C Adapter Project is distributed in the hope that
               it will be useful, but WITHOUT ANY WARRANTY; without even the
               implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
               PURPOSE.  See the GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with The 25 Cent I2C Adapter Project.  If not,
               see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <strings.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/i2c/IOI2CInterface.h>
#include "i2c.h"

/* At the moment this is rigged for a single I2C bus.  If support for multiple
   buses is desired in the future, some manner of non-OS-specific per-bus
   reference handle should be passed back to the calling code. */
static IOI2CConnectRef connect = NULL;

/****************************************************************************
 Function    : I2Copen()
 Description : Open connection to I2C bus associated with the last graphics
               controller on this system.
 Parameters  : None.
 Returns     : I2C_ERR_NONE on success, I2C_ERR_OPEN on error.
 ****************************************************************************/
int I2Copen()
{
	CFMutableDictionaryRef dict;

	/* Locate all available I2C buses */
	if((dict = IOServiceMatching(kIOI2CInterfaceClassName)))
	{
		io_iterator_t iterator = 0;

		/* Iterate through all items in dictionary.  dict is released
		   within IOServiceGetMatchingServices() so there's no need
		   for this code to do that. */
		(void)IOServiceGetMatchingServices(kIOMasterPortDefault,
		  dict,&iterator);

		if(iterator)
		{
			io_service_t last,interface = 0;

			/* We're interested only in the last interface in the
			   list; this will be the Mini-DVI port on systems
			   such as the MacBook or iMac (skipping past the
			   internal video interface), or the DVI port on
			   single-headed systems such as the Mac mini.
			   Multi-headed Mac Pro systems may require some
			   adjustments to the code to connect to the
			   preferred bus (or to access multiple I2C buses). */
			do {
				last = interface;
			} while((interface = IOIteratorNext(iterator)));

			(void)IOObjectRelease(iterator);

			if(last && (kIOReturnSuccess ==
			  IOI2CInterfaceOpen(last,kNilOptions,&connect)))
				return I2C_ERR_NONE;  /* Yay! */
		}
	}

	return I2C_ERR_OPEN;
}

/****************************************************************************
 Function    : I2Cmsg()
 Description : Issue I2C request and/or read response.
 Parameters  : short
               unsigned char *  Pointer to data to be sent (or NULL).
               int              Size of above data, in bytes (or 0).
               unsigned char *  Pointer to buffer to hold device response
                                data.  Pass NULL if no response is required
                                or anticipated.
               int              Size of response data, in bytes (0 if none).
 Returns     : 0 on success, else various OS-specific error codes.
 ****************************************************************************/
int I2Cmsg(
  short           const address,
  unsigned char * const sendBuf,
  int             const sendBytes,
  unsigned char * const replyBuf,
  int             const replyBytes)
{
	IOI2CRequest request;
	IOReturn     status;

	bzero(&request,sizeof(request));
	/* Of possible note here: optional callback function allows
	   the I2C request to operate asynchronously (non-blocking).
	   Keep in mind that request structure (and associated buffers)
	   would need to be persistent in that case. */
	request.completion  = NULL;
	/* Addressing seems to work a little wonky in OSX; I2C addresses are
	   supposed to be 7 bits, but it appears that the IOI2C* functions
	   include the subsequent read/write bit within the address fields,
	   making for an 8-bit value (address is shifted left by one).  But
	   now, the Mindsensors servo controller claims to have a default
	   address of 0xb0 (and even indicates such in its startup blink),
	   which exceeds the 7-bit address range.  I believe the servo
	   controller is treating the address and subsequent bit similarly.
	   So the left-shift isn't performed if the high bit is set. */
	request.sendAddress = request.replyAddress =
	  (address & 0x80) ? address : (address << 1);

	if(sendBuf && (sendBytes > 0))
	{
		request.sendTransactionType  = kIOI2CSimpleTransactionType;
		request.sendBuffer           = (vm_address_t)sendBuf;
		request.sendBytes            = sendBytes;
	}

	if(replyBuf && (replyBytes > 0))
	{
		request.replyTransactionType = kIOI2CSimpleTransactionType;
		request.replyBuffer          = (vm_address_t)replyBuf;
		request.replyBytes           = replyBytes;
	}

	status = IOI2CSendRequest(connect,kNilOptions,&request);

	return ((kIOReturnSuccess == status) && request.result) ?
	  request.result : (int)status;
}

/****************************************************************************
 Function    : I2Cclose()
 Description : Closes the previously-opened I2C bus connection.
 Parameters  : None.
 Returns     : None (void).
 ****************************************************************************/
void I2Cclose()
{
	if(connect)
	{
		(void)IOI2CInterfaceClose(connect,kNilOptions);
		connect = NULL;
	}
}
