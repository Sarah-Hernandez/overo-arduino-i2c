/****************************************************************************
 File        : servo.c
 Description : Example program for communicating with the Mindsensors I2C-SC8
               servo controller using the i2c.o library.  Unfortunately not
               100% reliable.  It appears that the DDC clock rate may be
               slightly more than this particular servo controller was
               designed for (primarily used with Lego Mindstorms and such).
               Other servo controllers may fare better (or worse).
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

#include <stdio.h>
#include <unistd.h>
#include "i2c.h"

/* See notes in platform-specific libraries regarding I2C-SC8 addressing.
   '0xb0' is technically not a valid 7-bit I2C address. */
static const short
	i2cAddress = 0xb0,  /* Default I2C address of I2C-SC8    */
	baseReg    = 0x02,  /* Servo #1 base register (low byte) */
	battReg    = 0x19;  /* Battery voltage register          */

/****************************************************************************
 Function    : moveServo()
 Description : Move one servo to the requested position.  Position is given
               in microsecond units as per servo PWM specification: 1500 is
               ostensibly centered, 1000 is minimum position, 2000 is maximum
               (some servos may have a narrower or wider range; these are
               just typical values).
 Parameters  : short  Servo number (0 - 7).
               short  Servo position (usec; 1000 - 2000).
 Returns     : int    0 on success, else various OS-specific error codes.
 Notes       : No range-checking of the input parameters is performed.
 ****************************************************************************/
static int moveServo(
  const short servoNum,
  const short position)
{
	unsigned char msg[3];

	msg[0] = baseReg + servoNum * 3;  /* Servo register on I2C-SC8 */
	msg[1] = position  & 0xff;        /* Low byte  (microseconds)  */
	msg[2] = position >> 8;           /* High byte (microseconds)  */

	return I2Cmsg(i2cAddress,msg,sizeof(msg),NULL,0);  /* No reply */
}

/****************************************************************************
 Function    : reportVoltage()
 Description : Request current servo battery voltage from the I2C-SC8
               (prints to standard output).
 Parameters  : None (void).
 Returns     : int  0 on success, else various OS-specific error codes.
 Notes       : This is somewhat baroque for a one byte request & reply,
               but is mostly meant to illustrate how one reads I2C replies
               (as opposed to the prior function, which is output only).
               Just change the size of sendBuf and replyBuf to deal with
               larger commands.
 ****************************************************************************/
static int reportVoltage(void)
{
	int           status;
	unsigned char msg[1],   /* Command to I2C-SC8 goes here       */
	              reply[1]; /* Reply from I2C-SC8 ends up here    */

	msg[0] = battReg;       /* Battery voltage address on I2C-SC8 */

	if(!(status = I2Cmsg(i2cAddress,msg,sizeof(msg),reply,sizeof(reply))))
	{
		/* 0.039 here is inferred from Mindsensors docmentation;
		   I2C-SC8 reports voltage in 39 millivolt increments. */
		(void)printf("Servo battery voltage: %0.2fV\n",
		  ((float)reply[0] * 0.039));
	}

	return status;
}

/****************************************************************************
 Function    : main()
 Description : Connects to I2C bus and issues calls to the prior functions,
               first printing the current servo battery voltage and then
               moving servo 0 through three repetitions of a four-position
               sequence.
 Parameters  : argc/argv currently ignored.
 Returns     : 0 on success, else various OS-specific error codes.
 ****************************************************************************/
int main(int argc,char *argv[])
{
	int status;

	if(I2C_ERR_NONE == (status = I2Copen()))
	{
		short i,p,pos[4] = { 1000,1500,2000,1500 };

		status = reportVoltage();

		for(i=0;(i<3)&&!status;i++)  /* Repeat sequence 3 times */
		{
			/* Step through positions, pausing 1 sec between */
			for(p=0;(p<(sizeof(pos)/sizeof(pos[0])))&&!status;p++)
			{
				(void)printf("Iteration %d, position %d\n",
				  i + 1,p + 1);
				status = moveServo(0,pos[p]);
				sleep(1);
			}
		}

		status = moveServo(0,0);  /* Disable servo 0 output */

		I2Cclose();
	}

	if(status) (void)printf("Exiting with error status %d\n",status);
	else       (void)puts("Exiting cleanly");

	return status;
}
