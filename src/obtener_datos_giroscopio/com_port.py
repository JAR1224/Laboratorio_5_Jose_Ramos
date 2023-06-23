import serial
import time
from threading import Thread
from threading import Event
import numpy as np

samples_per_gesture=70
number_of_gestures=100

ll = [0] * 100
print(ll)
ll = np.array(ll)
print(ll)
np.save('train/rest_output_noHot_0.npy',ll)

exit()

f = 'train/rest_accel_0.npy'

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

def send_string(tok):
    while(1):
        if event.is_set():
            break
        strr_ = input()
        serialPort.write(bytes(strr_,'utf-8'))
        print(event.is_set())

event = Event()
t = Thread(target=send_string, args=(1,) )
t.start()

count = 0
accel_arr = np.array([[0.1,0.1,0.1]])
gyro_arr = np.array([[0.1,0.1,0.1]])
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

            np.save(f,np.reshape(accel_arr[1:],(number_of_gestures,samples_per_gesture*2,3)))

            arr = np.load(f)
            print(np.shape(arr))

            event.set()
            t.join()
            exit()
