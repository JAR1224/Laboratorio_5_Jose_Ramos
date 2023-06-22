import serial
import time
from threading import Thread
import numpy as np

samples_per_gesture=70
number_of_gestures=2

#f2 = open('train/gyro.csv','w')
#f = open('train/accel.csv','w')

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

def send_string(tok):
    print("Thread started ",tok)
    while(1):
        strr_ = input()
        serialPort.write(bytes(strr_,'utf-8'))
        #f2.write(strr_.strip('\n'))

t = Thread(target=send_string, args=(1,) )
t.start()

#file_ = f
count = 0
accel_arr = np.array([[0.1,0.1,0.1]])
gyro_arr = np.array([[0.1,0.1,0.1]])
arr = accel_arr
bin = False

serialString = ""  # Used to hold data coming over UART
while 1:
    # Wait until there is data waiting in the serial buffer
    if serialPort.in_waiting > 0:

        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()
  

        # Print the contents of the serial data

        bin = True if bin == False else False
        strr = serialString.decode("Ascii")

        if (bin):
            temp_arr=[]
            temp = strr.strip("\n").strip("\r").split(",")
            for element in temp:
                temp_arr.append((float(element)+4)/8)
            temp_arr = [temp_arr]
            temp_arr = np.array(temp_arr).astype(np.float32)
            accel_arr =  np.concatenate((accel_arr,temp_arr))
        else:
            temp_arr=[]
            temp = strr.strip("\n").strip("\r").split(",")
            for element in temp:
                temp_arr.append((float(element)+2000)/40000)
            temp_arr = [temp_arr]
            temp_arr = np.array(temp_arr).astype(np.float32)
            accel_arr =  np.concatenate((accel_arr,temp_arr))

        count=count+1
        print(count,strr)
        if (count == number_of_gestures*samples_per_gesture*2):
            #arr1 = np.array(gyro_arr).astype(np.float)
            #arr2 = np.array(accel_arr).astype(np.float)
            #print(np.shape(gyro_arr[1:]))
            #print(np.shape(accel_arr[1:]))
            #print(gyro_arr)
            #print("========+++=======")
            #print(accel_arr)
            print("===============")
            #print(np.reshape(gyro_arr[1:],(number_of_gestures,samples_per_gesture,3)))
            #np.save('train/punch_gyro.npy',np.reshape(gyro_arr[1:],(number_of_gestures,samples_per_gesture,3)).astype(np.float32))

            np.save('train/punch_accel.npy',np.reshape(accel_arr[1:],(number_of_gestures,samples_per_gesture*2,3)))

            arr3 = np.load('train/punch_accel.npy')
            print(arr3)
            #arr4 = np.load('train/gyro.npy')
            #print(arr4)
            t.join()
            exit()
