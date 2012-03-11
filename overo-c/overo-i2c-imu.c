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

#define ARDUINO_I2C_ADDRESS 0x10
#define ARDUINO_I2C_BUFFER_LIMIT 32
#define I2C_BUS_DEVICE "/dev/i2c-3"

// Sensor I2C addresses
#define ACCEL_ADDRESS ((int) 0x53) // 0x53 = 0xA6 / 2
#define MAGN_ADDRESS  ((int) 0x1E) // 0x1E = 0x3C / 2
#define GYRO_ADDRESS  ((int) 0x68) // 0x68 = 0xD0 / 2

struct i2c_data_t {
  int fh;
  char buff[ARDUINO_I2C_BUFFER_LIMIT + 4];
  int len, sent, rcvd;
} i2c_data;

int gyro[3];
int accel[3];
int magnetom[3];

void i2cSend(char* msg){
  
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
  printf("Sent: %s\n", i2c_data.buff);
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

  if (i2c_data.rcvd > 0)
    printf("Received: %s\n", i2c_data.buff);
}

void i2cReciveBytes(int bytes){

  //clear buffer
  memset(i2c_data.buff, 0, sizeof(i2c_data.buff));
  
  //read	
  i2c_data.rcvd = read(i2c_data.fh, i2c_data.buff, bytes);
  
  while (i2c_data.rcvd < bytes) {
    
    usleep(50000);	
    i2c_data.len = read(i2c_data.fh, i2c_data.buff + i2c_data.rcvd, bytes - i2c_data.rcvd);

    if (i2c_data.len <= 0) {
      if (i2c_data.len < 0)
	perror("read");
      break;
    }
    
    i2c_data.rcvd += i2c_data.len;
  }

  if (i2c_data.rcvd > 0)
    printf("Received: %s\n", i2c_data.buff);
  
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
   // return 1;
  }
  
}

void i2cDestroy(){
  close(i2c_data.fh);
}

// Reads x, y and z gyroscope registers
void Read_Gyro()
{
  int i = 0;
  char buff[6];
  
  i2cSetAddress(GYRO_ADDRESS);
  
  // Sends address to read from
  i2cSend(0x1D);  
  
  // Request 6 bytes
  while(i<6){
    
    i2cReciveBytes(1);
    buff[i] = i2c_data.buff[0];   // Read one byte
    i++;
  }
  
  if (i == 6)  // All bytes received?
  {
    gyro[0] = -1 * ((((int) buff[2]) << 8) | buff[3]);    // X axis (internal sensor -y axis)
    gyro[1] = -1 * ((((int) buff[0]) << 8) | buff[1]);    // Y axis (internal sensor -x axis)
    gyro[2] = -1 * ((((int) buff[4]) << 8) | buff[5]);    // Z axis (internal sensor -z axis)
  }
  else
  {
    //num_gyro_errors++;
    //if (output_errors) 
      printf("!ERR: reading gyroscope");
  }
}

void Read_Magn()
{
  int i = 0;
  char buff[6];
 
  i2cSetAddress(MAGN_ADDRESS); 
  i2cSend(0x03);  // Send address to read from
  
  i2cSetAddress(MAGN_ADDRESS); 
  // Request 6 bytes
  while(i<6){ 
    
    i2cReciveBytes(1);
    buff[i] = i2c_data.buff[0];  // Read one byte
    i++;
  }
  
  if (i == 6)  // All bytes received?
  {
    // MSB byte first, then LSB; Y and Z reversed: X, Z, Y
    magnetom[0] = (((int) buff[0]) << 8) | buff[1];         // X axis (internal sensor x axis)
    magnetom[1] = -1 * ((((int) buff[4]) << 8) | buff[5]);  // Y axis (internal sensor -y axis)
    magnetom[2] = -1 * ((((int) buff[2]) << 8) | buff[3]);  // Z axis (internal sensor -z axis)
  }
  else
  {
    //num_magn_errors++;
    //if (output_errors)
      printf("!ERR: reading magnetometer");
  }
}

// Reads x, y and z accelerometer registers
void Read_Accel()
{
  int i = 0;
  char buff[6];
  
  i2cSetAddress(ACCEL_ADDRESS); 
  i2cSend(0x32);  // Send address to read from
  
  while(i<6)  // ((Wire.available())&&(i<6))
  {
    i2cReciveBytes(1);
    buff[i] = i2c_data.buff[0];  // Read one byte
    i++;
  }
  
  if (i == 6)  // All bytes received?
  {
    // No multiply by -1 for coordinate system transformation here, because of double negation:
    // We want the gravity vector, which is negated acceleration vector.
    accel[0] = (((int) buff[3]) << 8) | buff[2];  // X axis (internal sensor y axis)
    accel[1] = (((int) buff[1]) << 8) | buff[0];  // Y axis (internal sensor x axis)
    accel[2] = (((int) buff[5]) << 8) | buff[4];  // Z axis (internal sensor z axis)
  }
  else
  {
    //num_accel_errors++;
    //if (output_errors)
      printf("!ERR: reading accelerometer");
  }
}

void accel_init(){
  
  i2cSetAddress(ACCEL_ADDRESS);
  i2cSend(0x2D);  // Power register
  i2cSend(0x08);  // Measurement mode
  
  usleep(5000);
  
  i2cSetAddress(ACCEL_ADDRESS);
  i2cSend(0x31);  // Data format register
  i2cSend(0x08);  // Set to full resolution
  
  usleep(5000);
  
  // Because our main loop runs at 50Hz we adjust the output data rate to 50Hz (25Hz bandwidth)
  i2cSetAddress(ACCEL_ADDRESS);
  i2cSend(0x2C);  // Rate
  i2cSend(0x09);  // Set to 50Hz, normal operation

  usleep(5000);
}

void magn_init(){
  
  i2cSetAddress(MAGN_ADDRESS);
  i2cSend(0x02); 
  i2cSend(0x00);  // Set continuous mode (default 10Hz)
  
  usleep(5000);

  i2cSetAddress(MAGN_ADDRESS);
  i2cSend(0x00);
  i2cSend(0b00011000);  // Set 50Hz
  
  usleep(5000);
}

void gyro_init(){
  
 // Power up reset defaults
  i2cSetAddress(GYRO_ADDRESS);
  i2cSend(0x3E);
  i2cSend(0x80);
  usleep(5000);
  
  // Select full-scale range of the gyro sensors
  // Set LP filter bandwidth to 42Hz
  i2cSetAddress(GYRO_ADDRESS);
  i2cSend(0x16);
  i2cSend(0x1B);  // DLPF_CFG = 3, FS_SEL = 3
  usleep(5000);
  
  // Set sample rato to 50Hz
  i2cSetAddress(GYRO_ADDRESS);
  i2cSend(0x15);
  i2cSend(0x0A);  //  SMPLRT_DIV = 10 (50Hz)
  usleep(5000);

  // Set clock to PLL with z gyro reference
  i2cSetAddress(GYRO_ADDRESS);
  i2cSend(0x3E);
  i2cSend(0x00);
  usleep(5000);
  
}

int main(int argc, char **argv)
{
    i2cInit(I2C_BUS_DEVICE);
    i2cSetAddress(ARDUINO_I2C_ADDRESS);
    i2cSend(argv[1]);		
    i2cRecive();
    i2cDestroy();
    return 0;
}