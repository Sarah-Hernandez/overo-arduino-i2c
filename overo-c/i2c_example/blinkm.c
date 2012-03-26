/****************************************************************************
 File        : blinkm.c
 Description : Example program for communicating with a BlinkM "smart LED"
               using the i2c.o library.  Nothing fancy, just displays red,
               green and blue for one second each, repeating five times.  See
               the BlinkM datasheet for the full list of capabilities.  It's
               worth noting that the DDC specification calls for a minimum of
               50mA available current on the +5VDC line; more power may be
               available at the vendor's discretion, but is in no way
               guaranteed.  A fully-powered BlinkM (RGB all on, displaying
               white) will draw around 60mA, exceeding this stated limit.
               I've had no issue on the systems I've tested this with, but
               there's the possibility on other systems that the LED output
               may be diminished or, worst case, that the DDC host providing
               power may be overtaxed and perhaps even suffer damage.  So it
               might be prudent to provide separate power for the BlinkM.
               USE AT YOUR OWN RISK.
 History     : 5/3/2008  P. Burgess  Initial implementation
 Thanks to   : Tod Kurt of ThingM for entrusting me with one of his BlinkMs
               for testing at the 2008 Bay Area Maker Faire.
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
	i2cAddress = 0x09;  /* BlinkM default unless reprogrammed otherwise */

int main(int argc,char *argv[])
{
	int status;

	if(I2C_ERR_NONE == (status = I2Copen()))
	{
		int c,i;
		unsigned char
		  msgStop[]      = { 'o' },    /* Stop program on BlinkM */
		  msgColor[3][4] =
		  { { 'n',0xff,0x00,0x00 },    /* Immediate red   */
		    { 'n',0x00,0xff,0x00 },    /* Immediate green */
		    { 'n',0x00,0x00,0xff } } ; /* Immediate blue  */

		status = I2Cmsg(i2cAddress,msgStop,sizeof(msgStop),NULL,0);

		for(i=0;(I2C_ERR_NONE == status) && (i<5);i++)
		{
			for(c=0;(I2C_ERR_NONE == status) && (c<3);c++)
			{
				status = I2Cmsg(i2cAddress,msgColor[c],
				  sizeof(msgColor[c]),NULL,0);
				sleep(1);
			}
		}

		I2Cclose();
	}

	if(status) (void)printf("Exiting with error status %d\n",status);
	else       (void)puts("Exiting cleanly");

	return status;
}
