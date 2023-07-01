import serial

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)

serialString = ""  # Used to hold data coming over UART
while 1:
    # Esperar a que hayan datos seriales
    if serialPort.in_waiting > 0:

        # Leer hasta que llegue un caracter de nueva línea
        serialString = serialPort.readline()
  

        # Imprimir dato

        try:
            print(serialString.decode("Ascii"))
        except:
            pass
