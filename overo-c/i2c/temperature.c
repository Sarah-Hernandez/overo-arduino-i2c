/****************************************************************************
 File        : temperature.c
 Description : Example program for communicating with a Microchip TCN75A
               I2C temperature sensor using the i2c.o library.  Runs in an
               endless loop, printing the current temperature in degrees
               Centigrade and Farenheit about once per second.
 History     : 4/11/2008  P. Burgess  Initial implementation
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
	i2cAddress = 0x48,  /* 1001AAA as per Microchip specs */
	taReg      = 0x00,  /* Ambient temperature register   */
	configReg  = 0x01;  /* Configuration register         */

int main(int argc,char *argv[])
{
	int status;

	if(I2C_ERR_NONE == (status = I2Copen()))
	{
		unsigned char msg[2];

		/* Issue configuration command (2 bytes) */
		msg[0] = configReg;
		msg[1] = 0x60;      /* Enable 12-bit resolution */
		if(I2C_ERR_NONE == (status = I2Cmsg(i2cAddress,msg,2,NULL,0)))
		{
			unsigned char reply[2];

			/* Issue a series of temperature-reading commands
			   (1 byte each w/2-byte replies) */
			msg[0] = taReg;
			while(I2C_ERR_NONE == (status =
			  I2Cmsg(i2cAddress,msg,1,reply,2)))
			{
				float c,f;

				/* Bits 6-0 of first byte returned are integer
				   Centigrade temperature, bit 7 is sign
				   (negative if set); 1's complement format,
				   not 2's complement.  Bits 7-4 of second
				   byte are the fractional part of value. */
				c = (float)(reply[0] & 0x7f) +
				    (float)(reply[1] >> 4) / 16.0;
				if(reply[0] & 0x80) c = -c;

				/* Convert to Farenheit */
				f = 32.0 + c * (9.0 / 5.0);

				(void)printf("Temperature: %.2f°C %.2f°F\n",
				  c,f);

				/* Update once per second (roughly).
				   12-bit conversion takes about 240msec.
				   Allowing a little slop for I/O, sleep
				   for 3/4 sec. before next reading. */
				usleep(750000);
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
