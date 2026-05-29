# from sys import path
# path.append('./src')

import os
# Get the absolute path to the directory containing this script
# script_dir = os.path.dirname(os.path.abspath(__file__))

# Change the current working directory to that location
# os.chdir(script_dir)

print(os.getcwd())

import pfeiffer_controller
import socket
import time

controller = Controller("opc.tcp://192.168.1.100:4840")
turbo = Pump(controller, 1)
backing = Pump(controller, 2)

backing.pumping_power = True

time.wait(5)

backing.pumping_power = False