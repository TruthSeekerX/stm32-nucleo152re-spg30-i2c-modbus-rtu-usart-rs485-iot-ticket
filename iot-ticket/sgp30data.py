#lib
import json
import sys
import time
import random
import serial
from iotticket.models import device
from iotticket.models import criteria
from iotticket.models import deviceattribute
from iotticket.models import datanodesvalue
from iotticket.client import Client

#main

#data = json.load(open(sys.argv[1]))
data = json.load(open("config.json"))
username = data["username"]
password = data["password"]
deviceId = data["deviceId"]
baseurl = data["baseurl"]

c = Client(baseurl, username, password)

def send_data_to_iot_ticket(v1,v2):
    print("WRITE DEVICE DATANODES FUNTION.")
    listofvalues=[]
    nv1 = datanodesvalue()
    nv1.set_name("CO2eq") #needed for writing datanode
    nv1.set_path("co2eq")
    nv1.set_dataType("long")
    nv1.set_unit("ppm")
    nv1.set_value(v1) #needed for writing datanode
    nv1.set_timestamp(int(round(time.time() * 1000)))    # Have to explicitly set timestamp

    nv2 = datanodesvalue()
    nv2.set_name("TVOC") #needed for writing datanode
    nv2.set_path("tvoc")
    nv2.set_dataType("long")
    nv2.set_unit("ppb")
    nv2.set_value(v2) #needed for writing datanode
    nv2.set_timestamp(int(round(time.time() * 1000)))
    listofvalues.append(nv1)
    listofvalues.append(nv2)
    print(c.writedata(deviceId, *listofvalues)) # another way to make this would be c.writedata(deviceId, nv, nv1)
    print("END WRITE DEVICE DATANODES FUNCTION")
    print("-------------------------------------------------------\n")

while True:
    #write datanode demo
    
    ser_usart1 = serial.Serial('COM6', 9600, timeout=1)
    err = 0
    commands = {"Co2eq": bytes([5, 4, 0, 1, 0, 1, 142, 97]),
                "Tvoc": bytes([5, 4, 0, 2, 0, 1, 142, 145]),
                "BaseCo2eq": bytes([5, 4, 0, 3, 0, 1, 78, 192]),
                "BaseTvoc": bytes([5, 4, 0, 4, 0, 1, 143, 113])}
    ser_usart1.write(commands["Co2eq"])
    r = ser_usart1.read(10)
    err += r[1]
    co2eq = (r[3] << 8) + r[4]
    print("CO2eq:", co2eq)
    ser_usart1.write(commands["Tvoc"])
    r = ser_usart1.read(10)
    err += r[1]
    tvoc = (r[3] << 8) + r[4]
    print("tVOC:", tvoc)
    
    ser_usart1.write(commands["BaseCo2eq"])
    r = ser_usart1.read(10)
    err += r[1]
    baseCo2eq = (r[3] << 8) + r[4]
    print("Base CO2eq:", baseCo2eq)
    ser_usart1.write(commands["BaseTvoc"])
    r = ser_usart1.read(10)
    err += r[1]
    baseTvoc = (r[3] << 8) + r[4]
    print("Base tTVOC:", baseTvoc)
    
    if err == (4*4):    #some error checking here. Because if error reply function code is not 4 but 4 + 0x80 = 132
        send_data_to_iot_ticket(co2eq,tvoc)
    
    ser_usart1.close()

    print("sleeping 5s...")
    time.sleep(5)

