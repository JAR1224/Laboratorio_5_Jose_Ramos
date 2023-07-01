#include <Arduino_LSM9DS1.h>

#define NUM_SAMPLES 140
#define ACTIVITY_THRESHOLD 0.35
#define BUFFER_SIZE 20

// Función de valor absoluto
float abs_(float x) { return ((x)>0?(x):-(x)); }

//Declaración de variables
float aX, aY, aZ;
float gX, gY, gZ;
float data[(NUM_SAMPLES-BUFFER_SIZE)*6];
float buffer[BUFFER_SIZE*6];
float sum_ = 0;
int data_index = 0;
int buffer_index = 0;
int start_recording = 0;

void setup() {
  //Inicializar communicación serial
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  //Leer el primer dato del giroscopio
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);
  }
  delay(100);


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
        //Después de obtener todas las muestras, mandar el buffer y el arreglo a la PC serialmente
        data_index = 0;
        for (int i = 0 ; i < BUFFER_SIZE ; i++) {
          if (buffer_index == BUFFER_SIZE*6) {
            buffer_index = 0;
          }
          Serial.print(buffer[buffer_index++]);
          Serial.print(",");
          Serial.print(buffer[buffer_index++]);
          Serial.print(",");
          Serial.println(buffer[buffer_index++]);
          Serial.print(buffer[buffer_index++]);
          Serial.print(",");
          Serial.print(buffer[buffer_index++]);
          Serial.print(",");
          Serial.println(buffer[buffer_index++]);
        }
        for (int j = 0 ; j < (NUM_SAMPLES-BUFFER_SIZE) ; j++) {
          Serial.print(data[data_index++]);
          Serial.print(",");
          Serial.print(data[data_index++]);
          Serial.print(",");
          Serial.println(data[data_index++]);
          Serial.print(data[data_index++]);
          Serial.print(",");
          Serial.print(data[data_index++]);
          Serial.print(",");
          Serial.println(data[data_index++]);
        }
        data_index = 0;
        start_recording = 0;
      }
    }
  }

}
