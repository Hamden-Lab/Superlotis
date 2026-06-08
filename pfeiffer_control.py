from SuperLOTIS_v2.pfeiffer_controller import Controller, Pump
import socket
import time

url = "opc.tcp://192.168.1.100:4840"

controller = Controller(url)
controller.client.connect()

turbo = Pump(controller, "TC80", 1)
backing = Pump(controller, "MVP", 2)

# print(PUMP_PARAMETERS["MVP"]["pumping_power"])

backing.pumping_power = True

time.sleep(5)

backing.pumping_power = False

controller.client.disconnect()