
#ifndef GYRO_VEX
#define GYRO_VEX

#include <MPU6050_6Axis_MotionApps20.h>

class Gyro
{

private:
  //default address 0x68
  //MPU6050 mpu(0x69) alternative address
  MPU6050 mpu;

  // MPU control/status vars
  bool dmpReady;  // set true if DMP init was successful
  uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
  uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
  uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
  uint16_t fifoCount;     // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64]; // FIFO storage buffer

  // orientation/motion vars
  Quaternion q;           // [w, x, y, z]         quaternion container
  float euler[3];         // [psi, theta, phi]    Euler angle container
 

public:

  Gyro();
  bool init();
  void update();
  float getXDegrees();	



};


//indicates whether mpu interrupt pin has gone high
volatile bool mpuInterrupt = false;
void dmpDataReady() {
  mpuInterrupt = true;
}

Gyro::Gyro()
{
  dmpReady = false;
  Wire.begin();
}

bool Gyro::init()
{
  //init mpu
  devStatus  = mpu.dmpInitialize();  

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    //Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    //Serial.print(F("DMP Initialization failed (code "));
    //Serial.print(devStatus);
    //Serial.println(F(")"));
    return false;
  }

  return true;
}

void Gyro::update()
{
  
// wait for MPU interrupt or extra packet(s) available
   while (!mpuInterrupt && fifoCount < packetSize) {
   // other program behavior stuff here
   // .
   // .
   // .
   // if you are really paranoid you can frequently test in between other
   // stuff to see if mpuInterrupt is true, and if so, "break;" from the
   // while() loop to immediately process the MPU data
   // .
   // .
   // .
   
   }
   

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();


  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    // Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } 
  else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;



    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetEuler(euler, &q);
    //Serial.print("euler\t");
    //Serial.println(euler[0] * 180/M_PI);

  }

}

float Gyro::getXDegrees()
{  
  return (euler[0] * 180/M_PI + 180);
}


#endif GYRO_VEX



