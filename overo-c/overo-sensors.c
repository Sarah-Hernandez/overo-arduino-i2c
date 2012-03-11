
/*
  Simple I2C communication test with an Arduino as the slave device.
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#define I2C_DEVICE "/dev/i2c-3"
#define ARDUINO_I2C_ADDRESS 0x10
#define ARDUINO_I2C_BUFFER_LIMIT 32
#define ACCEL_ADDRESS ((int) 0x53) // 0x53 = 0xA6 / 2
#define MAGN_ADDRESS  ((int) 0x1E) // 0x1E = 0x3C / 2
#define GYRO_ADDRESS  ((int) 0x68) // 0x68 = 0xD0 / 2

static struct {
  int fh;
  char buff[ARDUINO_I2C_BUFFER_LIMIT + 4];
  int len, sent, rcvd;
} i2c_buffer i2c;

int i2cInit(){
  //open
  i2c.fh = open(I2C_DEVICE, O_RDWR);
  if (i2c.fh < 0) {
    perror("open");
    return -1;
  }else{
    return 1
  }
}


int i2cSetAddress(char* address){
  //set address
  if (ioctl(i2c.fh, I2C_SLAVE, address) < 0) {
    perror("ioctl");
    return -1;
  }
}

int main(int argc, char **argv){
  
  //copy stuff to send into buffer
  if (argc > 1) {
    memset(buff, 0, sizeof(buff));
    strncpy(buff, argv[1], ARDUINO_I2C_BUFFER_LIMIT);
  }
  else {
    strcpy(buff, "heartbeat");
  }
  
  i2cSetAddress(ARDUINO_I2C_ADDRESS);
  i2cSend(buff)
  
  
  close(i2c.fh);
  return 0;
}

int i2cSend(char* buff){
  
  //send
  len = strlen(buff);
  sent = write(fh, buff, len);
  	
  if (sent != len) {
    perror("write");
    return -1;
  }
  
  printf("Sent: %s\n", buff);
}

int i2cRecive(char* buffer){
  
  //clear buffer
  memset(buff, 0, sizeof(buff));
  //read	
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

  if (rcvd > 0)
    printf("Received: %s\n", buff);
}

int magRead(){
  
  
}

int gyroRead(){
  
  
}

int accRead(){
  
  
}