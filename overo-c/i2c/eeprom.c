/****************************************************************************
 File        : eeprom.c
 Description : Example code for programming and verifying a Microchip 24LC256
               I2C serial EEPROM using the i2c.o library.  Four tests are
               performed: a small sequential erase using one I2C transaction
               per byte, a small random address/random data write, a full
               device erase using per-page transactions, and a full device
               write (random data) using the same.  Similar EEPROMs are used
               in various novelty LED scroller signs such as the Best Buy
               "Roboshop" gift cards, so with some adjustments to match the
               specific EEPROM's protocol (see manufacturer's datasheet) this
               could be used as a method of downloading new messages to the
               sign.  *** USE WITH CAUTION! ***  Will overwrite the existing
               contents of the attached EEPROM, filling it with gibberish!
 History     : 4/6/2008  P. Burgess  Initial implementation
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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "i2c.h"

static const int
	romSize    = 32 * 1024;  /* Total chip capacity in bytes   */
static const short
	i2cAddress = 0x50,       /* 1010AAA as per Microchip specs */
	pageSize   = 64,         /* Bytes per page                 */
	testBytes  = 512;        /* Size of byte-at-a-time test    */

/****************************************************************************
 Function    : byteTest()
 Description : Write (and verify) data to/from EEPROM using a separate I2C
               transaction for each byte.
 Parameters  : int[]            Array of addresses to write (number of
                                elements must be equal to testBytes).
               unsigned char[]  Corresponding data values for each address.
               char[]           Description of operation being performed.
 Returns     : int              0 on success, else various OS-specific error
                                codes.
 Notes       : It is IMPERATIVE that each element in the address array be
               unique.  If two different values are written to one address,
               the verification process will fail.
               This method of writing is slow as heck for large volumes of
               data, hence the 512-byte test size.
 ****************************************************************************/
static int byteTest(
  const int           adr[],
  const unsigned char val[],
  const char          desc[])
{
	int           i = 0;
	unsigned char msg[3];

	(void)printf("Writing %d %s, byte-at-a-time",testBytes,desc);
	do {
		if(!(i & 31)) { (void)putchar('.'); fflush(stdout); }
		msg[0] = adr[i] >> 8;
		msg[1] = adr[i] & 0xff;
		msg[2] = val[i];
	} while((!I2Cmsg(i2cAddress,msg,3,NULL,0)) && (++i < testBytes));
	(void)puts((i < testBytes) ? "failed" : "OK");

	if(i >= testBytes)
	{
		unsigned char reply;

		(void)printf("Verifying");
		i = 0;
		do {
			if(!(i & 31)) { (void)putchar('.'); fflush(stdout); }
			msg[0] = adr[i] >> 8;
			msg[1] = adr[i] & 0xff;
			/* Initialize reply variable with an intentionally
			   wrong value to ensure that value returned by I2C
			   call is genuine (and not residue of a prior
			   invocation).  Helps avoid false positives. */
			reply  = val[i] ^ 0xff;
		} while((!I2Cmsg(i2cAddress,msg,2,&reply,1)) &&
		        (val[i] == reply) && (++i < testBytes));
		(void)puts((i < testBytes) ? "failed" : "OK");
	}

	return (i < testBytes);
}

/****************************************************************************
 Function    : pageTest()
 Description : Write (and verify) data to/from EEPROM using one I2C
               transaction for each memory page (typically 64 bytes).
 Parameters  : unsigned char[]  Memory map of entire device contents (Number
                                of elements must be equal to romSize).
 Returns     : int              0 on success, else various OS-specific error
                                codes.
 ****************************************************************************/
static int pageTest(
  const unsigned char map[],
  const char          desc[])
{
	int           i = 0;
	unsigned char msg[pageSize + 2];

	(void)printf("Writing %d %s, page-at-a-time",romSize,desc);
	do {
		if(!(i & 2047)) { (void)putchar('.'); fflush(stdout); }
		msg[0] = i >> 8;
		msg[1] = i & 0xff;
		(void)memcpy(&msg[2],&map[i],pageSize);
	} while((!I2Cmsg(i2cAddress,msg,sizeof(msg),NULL,0)) &&
	        ((i += pageSize) < romSize));
	(void)puts((i < romSize) ? "failed" : "OK");

	if(i >= romSize)
	{
		unsigned char reply[pageSize];

		(void)printf("Verifying");
		i = 0;
		do {
			if(!(i & 2047)) { (void)putchar('.'); fflush(stdout); }
			msg[0] = i >> 8;
			msg[1] = i & 0xff;
			/* As with byte case, avoid false positives */
			memset(reply,0xff,pageSize);
		} while((!I2Cmsg(i2cAddress,msg,2,reply,sizeof(reply))) &&
		        (!memcmp(reply,&map[i],pageSize)) &&
		        ((i += pageSize) < romSize));
		(void)puts((i < romSize) ? "failed" : "OK");
	}

	return (i < romSize);
}

/****************************************************************************
 Function    : main()
 Description : Connects to I2C bus and issues calls to the prior functions,
               exercising the attached EEPROM in various ways.
 Parameters  : argc/argv currently ignored.
 Returns     : 0 on success, else various OS-specific error codes.
 ****************************************************************************/
int main(int argc,char *argv[])
{
	int status;

	if(I2C_ERR_NONE == (status = I2Copen()))
	{
		int           adr[testBytes],i;
		unsigned char val[testBytes];

		srand(time(NULL));

		/* Set up linear range of addresses, clear each */
		for(i=0;i<testBytes;i++) adr[i] = i;
		bzero(val,sizeof(val));
		if(!(status = byteTest(adr,val,"sequential zeros")))
		{
		  /* Sequential zero test completed successfully.
		     Set up random range of addresses and random values */
		  int a,j;
		  for(i=0;i<testBytes;i++)
		  {
		    /* Ensure each address is unique, else verify may fail */
		    do {
		      a = rand() % romSize;
		      for(j=0;(j<i) && (a != adr[j]);j++);
		    } while(j < i);
		    adr[i] = a;
		    val[i] = rand() & 0xff;
		  }
		  if(!(status = byteTest(adr,val,"random addresses/values")))
		  {
		    /* Random address/data test was successful.
		       Clear entire memory map and write to device */
		    unsigned char map[romSize];
		    bzero(map,sizeof(map));
		    if(!(status = pageTest(map,"zeros")))
		    {
		      /* Full device clear was successful.
		         Fill with and verify random data. */
		      for(i=0;i<romSize;i++) map[i] = rand() & 0xff;
		      status = pageTest(map,"random values");
		    }
		  }
		}

		I2Cclose();
	}

	if(status) (void)printf("Exiting with error status %d\n",status);
	else       (void)puts("Exiting cleanly");

	return status;
}
