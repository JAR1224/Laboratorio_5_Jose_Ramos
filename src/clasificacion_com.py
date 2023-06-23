import serial
import time
from threading import Thread
from threading import Event
import numpy as np

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


serialString = ""  # Used to hold data coming over UART
while 1:
    # Wait until there is data waiting in the serial buffer
    if serialPort.in_waiting > 0:

        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()
  

        # Print the contents of the serial data

        try:
            print(serialString.decode("Ascii"))
        except:
            pass
