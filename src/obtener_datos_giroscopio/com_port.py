import serial
import time
import numpy as np

samples_per_gesture=140
number_of_gestures=100

gesture = 2 #knee
gestures = ["punch","jump","knee"]

f = gestures[gesture] + '_accel_0.npy'

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

count = 0
accel_arr = np.array([[0.1,0.1,0.1]])
gyro_arr = np.array([[0.1,0.1,0.1]])
bin = False

serialString = "" 
while 1:
    # Esperar hasta que lleguen datos seriales
    if serialPort.in_waiting > 0:

        # Leer datos seriales hasta el caracter de nueva línea
        serialString = serialPort.readline()

        # Alternar entre datos de aceleración y datos de giroscopio
        bin = True if bin == False else False
        strr = serialString.decode("Ascii")

        # Guardar datos de aceleración en arreglo
        if (bin):
            temp_arr=[]
            temp = strr.strip("\n").strip("\r").split(",")
            for element in temp:
                temp_arr.append((float(element)+4)/8)
            temp_arr = [temp_arr]
            temp_arr = np.array(temp_arr).astype(np.float32)
            accel_arr =  np.concatenate((accel_arr,temp_arr))
        # Guardar datos de giroscopio en arreglo
        else:
            temp_arr=[]
            temp = strr.strip("\n").strip("\r").split(",")
            for element in temp:
                temp_arr.append((float(element)+2000)/4000)
            temp_arr = [temp_arr]
            temp_arr = np.array(temp_arr).astype(np.float32)
            accel_arr =  np.concatenate((accel_arr,temp_arr))

        count=count+1
        print(count/(samples_per_gesture*2),strr)

        # Si ya se obtuvo todas las muestras esperadas, guardar en un archivo .npy
        if (count == number_of_gestures*samples_per_gesture*2):

            np.save(f,np.reshape(accel_arr[1:],(number_of_gestures,samples_per_gesture*2,3)))

            arr = np.load(f)
            print(np.shape(arr))

            # Guardar en otro archivo .npy las salidas
            ll = [gesture] * number_of_gestures
            ll = np.array(ll)
            np.save(gestures[gesture] + '_output_noHot_0.npy',ll)
            exit()
