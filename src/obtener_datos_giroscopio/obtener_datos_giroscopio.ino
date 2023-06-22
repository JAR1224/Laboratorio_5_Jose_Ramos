#include <Arduino_LSM9DS1.h>
//#include <TensorFlowLite.h>

#define NUM_SAMPLES 70

float aX, aY, aZ;
float gX, gY, gZ;
int samples_per_gesture = NUM_SAMPLES;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print out the samples rates of the IMUs
  //Serial.print("Accelerometer sample rate = ");
  //Serial.print(IMU.accelerationSampleRate());
  //Serial.println(" Hz");
  //Serial.print("Gyroscope sample rate = ");
  //Serial.print(IMU.gyroscopeSampleRate());
  //Serial.println(" Hz");

  //delay(20000);

}

void loop() {
  if (samples_per_gesture == NUM_SAMPLES) {
    if (Serial.available() > 0) {
      Serial.read();
      samples_per_gesture=0;
      delay(1000);
    }
  } else {

    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(aX, aY, aZ);
      Serial.print(aX);
      Serial.print(",");
      Serial.print(aY);
      Serial.print(",");
      Serial.println(aZ);
    }

    if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(gX, gY, gZ);
      Serial.print(gX);
      Serial.print(",");
      Serial.print(gY);
      Serial.print(",");
      Serial.println(gZ);
    }
    samples_per_gesture++;
  }

}
