#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>

#include "constants.h"
#include "main_functions.h"
#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

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
