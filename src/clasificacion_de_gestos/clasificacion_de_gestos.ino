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

namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 60 * 1024;
// Keep aligned to 16 bytes for CMSIS
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

float aX, aY, aZ;
float gX, gY, gZ;
int samples_per_gesture = 0;

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

  tflite::InitializeTarget();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided is schema version x not equal to supported version x.");
    while (1);
    return;
  }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    while(1);
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  inference_count = 0;

}

float gest_1,gest_2;
int condition;

void loop() {
  if (samples_per_gesture == NUM_SAMPLES) {

    if (Serial.available() > 0) {
      Serial.read();
      samples_per_gesture=0;

      TfLiteStatus invokeStatus = interpreter->Invoke();
      if (invokeStatus != kTfLiteOk) {
        Serial.println("Invoke failed!");
        while (1);
        return;
      }

      // Loop through the output tensor values from the model
      //for (int i = 0; i < 2; i++) {
        //Serial.print(GESTURES[i]);
      //  Serial.print(": ");
      //  Serial.println(output->data.f[i], 6);
      //}
      Serial.println(output->data.f[0] > output->data.f[1] ? "estacionario" : "punetazo");

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

    input->data.f[samples_per_gesture * 6 + 0] = (aX + 4.0) / 8.0;
    input->data.f[samples_per_gesture * 6 + 1] = (aY + 4.0) / 8.0;
    input->data.f[samples_per_gesture * 6 + 2] = (aZ + 4.0) / 8.0;
    input->data.f[samples_per_gesture * 6 + 3] = (gX + 2000.0) / 40000.0;
    input->data.f[samples_per_gesture * 6 + 4] = (gY + 2000.0) / 40000.0;
    input->data.f[samples_per_gesture * 6 + 5] = (gZ + 2000.0) / 40000.0;

    samples_per_gesture++;

  }

}