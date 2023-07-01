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

#define NUM_SAMPLES 140
#define ACTIVITY_THRESHOLD 0.35
#define BUFFER_SIZE 20

// Función de valor absoluto
float abs_(float x) { return ((x)>0?(x):-(x)); }

namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
//int inference_count = 0;

constexpr int kTensorArenaSize = 60 * 1024;

alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}  

//Declaración de variables
float aX, aY, aZ;
float gX, gY, gZ;
float data[(NUM_SAMPLES-BUFFER_SIZE)*6];
float buffer[BUFFER_SIZE*6];
float sum_ = 0;
int data_index = 0;
int buffer_index = 0;
int start_recording = 0;
int input_index = 0;

void setup() {
  // Inicializar comunicación serial con la PC
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

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

  input = interpreter->input(0);
  output = interpreter->output(0);

}

void loop() {

  //Esperar hasta que haya suficiente actividad en el giroscopio
  if (sum_ > ACTIVITY_THRESHOLD) {
    start_recording = 1;
    sum_ = 0;
  }

  //Mientras no haya suficiente actividad:
  if (start_recording == 0) {
    if (buffer_index == BUFFER_SIZE*6) {
      buffer_index = 0;
    }
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      //Leer, normalizar y sumar datos del giroscopio. Guardar datos en un buffer
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);
      sum_ = 0;
      buffer[buffer_index] = aX;
      sum_ += abs_(buffer[buffer_index++])/4;
      buffer[buffer_index] = aY;
      sum_ += abs_(buffer[buffer_index++])/4;
      buffer[buffer_index] = aZ;
      sum_ += abs_(buffer[buffer_index++])/4;
      buffer[buffer_index] = gX;
      sum_ += abs_(buffer[buffer_index++])/2000;
      buffer[buffer_index] = gY;
      sum_ += abs_(buffer[buffer_index++])/2000;
      buffer[buffer_index] = gZ;
      sum_ += abs_(buffer[buffer_index++])/2000;
    }
  //Si se detecta suficiente actividad:
  } else if (start_recording == 1) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      //Leer y guardar datos del giroscopio en arreglo
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);
      data[data_index++] = aX;
      data[data_index++] = aY;
      data[data_index++] = aZ;
      data[data_index++] = gX;
      data[data_index++] = gY;
      data[data_index++] = gZ;
      if (data_index == (NUM_SAMPLES-BUFFER_SIZE)*6) {
        //Después de obtener todas las muestras (buffer + arreglo), realizar la inferencia y mandar resultados a la PC
        data_index = 0;
        input_index = 0;
        for (int i = 0 ; i < BUFFER_SIZE ; i++) {
          if (buffer_index == BUFFER_SIZE*6) {
            buffer_index = 0;
          }
          input->data.f[input_index * 6 + 0] = (buffer[buffer_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 1] = (buffer[buffer_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 2] = (buffer[buffer_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 3] = (buffer[buffer_index++] + 2000.0) / 4000.0;
          input->data.f[input_index * 6 + 4] = (buffer[buffer_index++] + 2000.0) / 4000.0;
          input->data.f[input_index * 6 + 5] = (buffer[buffer_index++] + 2000.0) / 4000.0;
          input_index++;
        }
        for (int j = 0 ; j < (NUM_SAMPLES-BUFFER_SIZE) ; j++) {
          input->data.f[input_index * 6 + 0] = (data[data_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 1] = (data[data_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 2] = (data[data_index++] + 4.0) / 8.0;
          input->data.f[input_index * 6 + 3] = (data[data_index++] + 2000.0) / 4000.0;
          input->data.f[input_index * 6 + 4] = (data[data_index++] + 2000.0) / 4000.0;
          input->data.f[input_index * 6 + 5] = (data[data_index++] + 2000.0) / 4000.0;
          input_index++;
        }
        data_index = 0;
        start_recording = 0;
        TfLiteStatus invokeStatus = interpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }
        Serial.println(output->data.f[0] > output->data.f[1] ? ( output->data.f[0] > output->data.f[2] ? "punetazo" : "rodilla" ) : ( output->data.f[1] > output->data.f[2] ? "salto" : "ropdilla" ) );
      }
    }
  }

}