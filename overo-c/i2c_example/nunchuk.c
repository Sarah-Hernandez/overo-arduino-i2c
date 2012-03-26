/****************************************************************************
 File        : nunchuk.c
 Description : Example program for reading the output from a Nintendo Wii
               "Nunchuk" controller accessory using the i2c.o library.  Runs
               in an endless loop, printing the raw output of the X, Y and Z
               axis accelerometers, analog joystick and trigger buttons.
               Note that the Nunchuk and other Wii remote accessories are
               ostensibly all 3 volt devices, while DDC provides 5 volt
               power and signaling.  Many folks (self included) have reported
               success operating the Nunchuk at 5 volts, but as always I must
               include the "at your own risk" disclaimer here.
               DOES NOT CURRENTLY WORK ON MAC!  Not yet sure why.  Linux OK.
 History     : 5/23/2008  P. Burgess  Initial implementation
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

#include <stdio.h>
#include <unistd.h>
#include "i2c.h"

static const short
	i2cAddress = 0x52;  /* Nunchuk addr.  Do all accessories use same? */

int main(int argc,char *argv[])
{
	int status;

	if(I2C_ERR_NONE == (status = I2Copen()))
	{
		unsigned char msgInit[] = { 0x40,0x00 },
		              msgReq[]  = { 0x00 };

		if(I2C_ERR_NONE == (status = I2Cmsg(i2cAddress,msgInit,
		  sizeof(msgInit),NULL,0)))
		{
			int           ax,ay,az;  /* Accelerometer values */
			unsigned char reply[6];
			const char   *buttons[] = { "-Z","C-","CZ","--" };

			while(I2C_ERR_NONE == (status = I2Cmsg(i2cAddress,
			  msgReq,sizeof(msgReq),reply,sizeof(reply))))
			{

				ax = (reply[2] << 2) | ((reply[5] >> 2) & 3);
				ay = (reply[3] << 2) | ((reply[5] >> 4) & 3);
				az = (reply[4] << 2) | ((reply[5] >> 6) & 3);
				(void)printf("Stick: %d %d  Buttons: %s  "
				  "Acceleration: %d %d %d\n",reply[0],reply[1],
				  buttons[reply[5] & 3],ax,ay,az);
			}
		}

		I2Cclose();
	}

	/* Technically the clean exit case here will never happen (program
	   runs indefinitely until interrupted or I2Cmsg() fails), but it's
	   left here in case you want to change the program to perform a
	   single reading or some other variation. */
	if(status) (void)printf("Exiting with error status %d\n",status);
	else       (void)puts("Exiting cleanly");

	return status;
}
