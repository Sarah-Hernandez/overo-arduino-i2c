/*
  Overi2c.cpp - i2c library to access i2c devices from userspace
  
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C" {
  
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  //#include "twi.h"

}

#include "Overi2c.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t Overi2c::rxBuffer[BUFFER_LENGTH];
uint8_t Overi2c::rxBufferIndex = 0;
uint8_t Overi2c::rxBufferLength = 0;

uint8_t Overi2c::txAddress = 0;
uint8_t Overi2c::txBuffer[BUFFER_LENGTH];
uint8_t Overi2c::txBufferIndex = 0;
uint8_t Overi2c::txBufferLength = 0;

uint8_t Overi2c::transmitting = 0;
void (*Overi2c::user_onRequest)(void);
void (*Overi2c::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////
Overi2c::Overi2c()
{
}

// Public Methods //////////////////////////////////////////////////////////////
void Overi2c::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  //open device <-- must be read from a config file
  fh = open("/dev/i2c-3", O_RDWR);
  if (fh < 0) {
    perror("open");
    return 1;
  }
  
}

void Overi2c::begin(uint8_t address)
{
  //twi_setAddress(address);
  
  //set address	
    if(ioctl(fh, I2C_SLAVE, ARDUINO_I2C_ADDRESS) < 0) {
      
      perror("ioctl");
      return 1;
  }
  
  //don't know how to use interrupts yet.
  //don't need this when in master mode ?
  
  //twi_attachSlaveTxEvent(onRequestService);
  //twi_attachSlaveRxEvent(onReceiveService);
  
  //open file handle
  begin();
}

void Overi2c::begin(int address)
{
  begin((uint8_t)address);
}


uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{

  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  
  // perform blocking read into buffer
  //uint8_t read = twi_readFrom(address, rxBuffer, quantity);
  rcvd = read(fh, buff, sent);
  while (rcvd < sent) {
    usleep(50000);	
    len = read(fh, buff + rcvd, sent - rcvd);
    if (len <= 0) {
      if (len < 0)
      perror("read");
      break;
    }
    rcvd += len;
  }
  
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity);
}

void Overi2c::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  
  // set address of targeted slave
  txAddress = address;
  
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void Overi2c::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

uint8_t Overi2c::endTransmission(void)
{
  // transmit buffer (blocking)
  // int8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1);
  
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  
  // indicate that we are done transmitting
  transmitting = 0;
  return ret;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      setWriteError();
      return 0;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
  }else{
    // in slave send mode
    // reply to master
    twi_transmit(&data, 1);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  if(transmitting){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(data, quantity);
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void)
{
  int value = -1;
  
  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void)
{
  int value = -1;
  
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
  }

  return value;
}

void TwoWire::flush(void)
{
  // XXX: to be implemented.
}

// behind the scenes function that is called when data is received
void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];    
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void TwoWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;

  // alert user program
  user_onRequest();
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

// Preinstantiate Objects //////////////////////////////////////////////////////
TwoWire Wire = TwoWire();

