/*
  Simple I2C communication test with an Arduino as the slave device.
*/

#include "overo-i2c.h"

void i2cSendByte(int msg){
  
  printf("sending 0x%X\n", msg);  
    //send
    i2c_data.sent = write(i2c_data.fh, &msg, 1);
    
    //verify sent
    if (i2c_data.sent != 1) {
      perror("write");
      //printf("& the buffer is messed up\n");
    }
}

void i2cSend(char* msg){
  
  //printf("sending %s\n", msg);
  
  //copy stuff to send into buffer
  if (sizeof(msg) > 1) {

      memset(i2c_data.buff, 0, sizeof(i2c_data.buff));
      strncpy(i2c_data.buff, msg, ARDUINO_I2C_BUFFER_LIMIT);
  }
  else {
    strcpy(i2c_data.buff,"hello");
  }
  
  //send
  i2c_data.len = strlen(i2c_data.buff);
  i2c_data.sent = write(i2c_data.fh, i2c_data.buff, i2c_data.len);

  if (i2c_data.sent != i2c_data.len) {
    perror("write");
    //return 1;
  }
  //printf("Sent: %s\n", i2c_data.buff);
}

void i2cRecive(){

  //clear buffer
  memset(i2c_data.buff, 0, sizeof(i2c_data.buff));
  
  //read	
  i2c_data.rcvd = read(i2c_data.fh, i2c_data.buff, i2c_data.sent);
  while (i2c_data.rcvd < i2c_data.sent) {
    
    usleep(50000);	
    i2c_data.len = read(i2c_data.fh, i2c_data.buff + i2c_data.rcvd, i2c_data.sent - i2c_data.rcvd);

    if (i2c_data.len <= 0) {
      if (i2c_data.len < 0)
	perror("read");
      break;
    }
    i2c_data.rcvd += i2c_data.len;

  }

  if (i2c_data.rcvd > 0){
    //printf("Received: %s\n", i2c_data.buff);
  }
    
}

void i2cReciveBytes(int bytes){

  //clear buffer
  memset(i2c_data.buff, 0, sizeof(i2c_data.buff));
  
  //read	
  i2c_data.rcvd = read(i2c_data.fh, i2c_data.buff, bytes);
  
  while (i2c_data.rcvd < bytes) {
    
    usleep(5000);	
    i2c_data.len = read(i2c_data.fh, i2c_data.buff + i2c_data.rcvd, bytes - i2c_data.rcvd);

    if (i2c_data.len <= 0) {
      if (i2c_data.len < 0)
	perror("read");
      break;
    }
    
    i2c_data.rcvd += i2c_data.len;
  }

  if (i2c_data.rcvd > 0){
    //printf("Received: %s\n", i2c_data.buff);
  }
  else{
    printf("ERR!: Recived Error");
  }
}

void i2cSetAddress(int address){
  
  //set address
  if (ioctl(i2c_data.fh, I2C_SLAVE, address) < 0) {
    perror("ioctl");
   // return 1;
  }
}

void i2cInit(char* i2c_device){
  
  //open device
  i2c_data.fh = open(i2c_device, O_RDWR);
  if (i2c_data.fh < 0) {
    perror("open");
  }
  
}

void i2cDestroy(){
  close(i2c_data.fh);
}

