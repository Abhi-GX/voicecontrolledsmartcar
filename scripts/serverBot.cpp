#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL
int rr,ll,yaw;
int const INTERRUPT_PIN = 2;  // Define the interruption #0 pin
bool blinkState;
bool DMPReady = false;  // Set true if DMP init was successful
uint8_t MPUIntStatus;   // Holds actual interrupt status byte from MPU
uint8_t devStatus;      // Return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // Expected DMP packet size (default is 42 bytes)
uint8_t FIFOBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         Quaternion container
VectorInt16 aa;         // [x, y, z]            Accel sensor measurements
VectorInt16 gy;         // [x, y, z]            Gyro sensor measurements
VectorInt16 aaReal;     // [x, y, z]            Gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            World-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            Gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   Yaw/Pitch/Roll container and gravity vector
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };
volatile bool MPUInterrupt = false;     // Indicates whether MPU6050 interrupt pin has gone high
void DMPDataReady() {
  MPUInterrupt = true;
}

bool r=false,l=false,r1=false,l1=false;
unsigned long timer;  // global declaration
int trigPin = 7;    // Trigger
int echoPin = 8;    // Echo
bool k=false;
long duration, cm, inches;
long duration1, cm1, inches1;
int a=1;
int b=43;
int c=3;
int d=4;
int e=9;
int f=44;
const char* ssid = "KMIT_AUDL";
const char* password = "123456789";
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 180;
WebServer server(80);
void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP32!");
}

void handlePost() {
  String message = server.arg("message");
  Serial.println(message);
  if(message=="on"){
    digitalWrite(LED_BUILTIN,HIGH);
    server.send(200, "text/plain","LED ON");
  }else if(message=="off"){
    digitalWrite(LED_BUILTIN,LOW);
    server.send(200, "text/plain","LED OFF");
  }else if(message=="f")
  {
    k=true;
    ledcWrite(a, dutyCycle);
    ledcWrite(d, dutyCycle);
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    digitalWrite(e, HIGH);
    digitalWrite(f, LOW);
    server.send(200, "text/plain","going forward");
  }
  else if(message=="x")
  {
    ledcWrite(a, dutyCycle);
    ledcWrite(d, dutyCycle);
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(e, LOW);
    digitalWrite(f, HIGH);
    server.send(200, "text/plain","going backward");
  }
  else if(message=="r")
  {
    l=false;
    r=true;
    r1=true;
    l1=false;
    server.send(200, "text/plain","rotating right");
  }
  else if(message=="l")
  {
    r=false;
    l=true;
    r1=false;
    l1=true;
    server.send(200, "text/plain","rotating left");
  }
  else if(message=="b")
  {
    r=false;
    r1=false;
    l=false;
    l1=false;
    ledcWrite(a, 0);
    ledcWrite(d, 0);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    server.send(200, "text/plain","break applied");
  }
  else
  {
    server.send(200,"text/plain","Invalid");
  }
}
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/post", HTTP_POST, handlePost);
  server.begin();
  Serial.println("Server started");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  ledcAttachChannel(a, freq, resolution, pwmChannel);
  ledcAttachChannel(d, freq, resolution, pwmChannel);
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment on this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif
  while (!Serial);

  /*Initialize device*/
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  /*Verify connection*/
  Serial.println(F("Testing MPU6050 connection..."));
  if(mpu.testConnection() == false){
    Serial.println("MPU6050 connection failed");
    while(true);
  }
  else {
    Serial.println("MPU6050 connection successful");
  }
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  /* Supply your gyro offsets here, scaled for min sensitivity */
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);

  /* Making sure it worked (returns 0 if so) */ 
  if (devStatus == 0) {
    mpu.CalibrateAccel(6);  // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateGyro(6);
    Serial.println("These are the Active offsets: ");
    mpu.PrintActiveOffsets();
    Serial.println(F("Enabling DMP..."));   //Turning ON DMP
    mpu.setDMPEnabled(true);

    /*Enable Arduino interrupt detection*/
    Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
    Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    Serial.println(F(")..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), DMPDataReady, RISING);
    MPUIntStatus = mpu.getIntStatus();

    /* Set the DMP Ready flag so the main loop() function knows it is okay to use it */
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    DMPReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize(); //Get expected DMP packet size for later comparison
  } 
  else {
    Serial.print(F("DMP Initialization failed (code ")); //Print the error code
    Serial.print(devStatus);
    Serial.println(F(")"));
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
  }
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // if programming failed, don't try to do anything
    server.handleClient();
    if (!DMPReady) return; // Stop the program if DMP programming fails.
    if (mpu.dmpGetCurrentFIFOPacket(FIFOBuffer)) { // Get the Latest packet 
    #ifdef OUTPUT_READABLE_YAWPITCHROLL
      /* Display Euler angles in degrees */
      mpu.dmpGetQuaternion(&q, FIFOBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      yaw=ypr[0] * 180/M_PI;
    #endif
  }
  if(r1==true)
  {
    if((yaw >= 0)&&(yaw <= 90))
    {
      rr=yaw-90;
    }
    else if((yaw > 90)&&(yaw <= 179))
    {
      rr=yaw-90;
    }
    else if((yaw >= -179)&&(yaw <= -90))
    {
      rr=yaw+268;
    }
    else if((yaw >- 90)&&(yaw < 0))
    {
      rr=yaw-90;
    }
    ledcWrite(a, 150);
    ledcWrite(d, 150);
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(e, HIGH);
    digitalWrite(f, LOW); 
    r1=false;
  }
  if(l1==true)
  {
    if((yaw >= 0)&&(yaw <= 90))
    {
      rr=yaw+90;
    }
    else if((yaw > 90)&&(yaw <= 179))
    {
      rr=yaw-268;
    }
    else if((yaw >= -179)&&(yaw <= -90))
    {
      rr=yaw+90;
    }
    else if((yaw >-90)&&(yaw < 0))
    {
      rr=yaw+90;
    }
    ledcWrite(a, 150);
    ledcWrite(d, 150);
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, HIGH); 
    l1=false;
  }
  if(l==true)
    {
      Serial.println(yaw);
        Serial.print("  ");
        Serial.print(rr);
      if(yaw==rr)
      {
        ledcWrite(a, 0);
    ledcWrite(d, 0);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    l=false;
      }
    }
    if(r==true)
    {
      Serial.println(yaw);
        Serial.print("  ");
        Serial.print(rr);
      if(yaw==rr)
      {
        ledcWrite(a, 0);
    ledcWrite(d, 0);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    r=false;
      }
    }
  if(k==true){
    timer = millis();
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  cm = (duration/2) / 29.1;
  Serial.println(cm);
  if(cm<20){
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  ledcWrite(a, 0);
  ledcWrite(d, 0);
  k=false;
  }
  delay(100);
  }
}