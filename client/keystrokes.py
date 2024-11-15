#!/usr/bin/python
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2018 Sony Corporation
#
# Author: Tim Bird <tim.bird@sony.com>
#
# This assumes that the teensy has the etherkey sketch loaded,
# and that the serial port (pins gnd,0,1) are connected to the
# local machine via some device (e.g. USB serial, in the examples
# below)
#

import serial
import sys
import time

def configure_serial(port, baudrate):
    """
    configure serial port and open the port
    
    parameters:
    port -- serial port number
    baudrate
    return:
    ser -- configured serial port
    """
    ser = serial.Serial(timeout=1)
    ser.port = port  # serial port
    ser.baudrate = baudrate  # baud rate
    # ser.bytesize = serial.EIGHTBITS  # digital byte size
    # ser.parity = serial.PARITY_NONE  # parity bit
    # ser.stopbits = serial.STOPBITS_ONE  # stop bit

    # open serial port
    ser.open()
    if ser.isOpen():
        print("The serial port is enabled.")
    else:
        print("The serial port can't be enabled, please check the configuration of serial port!")
        return None

    return ser


def receive_data(port):
    if port.in_waiting > 0:
        response = port.readline().decode().strip()
        return response
    return None

def send_data(message):
    try:
    # make sure we're in Command mode
    # send ctrl-Q, then 1
    # cmd=bytes([17])
        ser.write(message.encode())
        while True:
            if ser.in_waiting > 0:
                response = receive_data(ser)
                if response:
                    print(f"Received: {response}")
                else:
                    print("No data received.")
            else:
                break
    except serial.SerialException as e:
        print(f"Error: {e}")    

device = "/dev/ttyUSB0"
ser = configure_serial(device, 57600)
time.sleep(1)

message = '\x11'
send_data(message)
time.sleep(0.1)

message = "1"
send_data(message)
time.sleep(0.1)

cmd="a"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)
time.sleep(0.1)

cmd="up"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)

cmd="space"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)
time.sleep(0.1)

cmd="a"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)
time.sleep(0.1)

cmd="space"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)
time.sleep(0.1)

cmd="B"
message = "Send {{{}}}\n\r".format(cmd)
send_data(message)
time.sleep(0.1)
# print(f"Port: {ser.port}")
# print(f"Baud rate: {ser.baudrate}")
# print(f"Tineout: {ser.timeout}")
# print(f"Stop bits: {ser.stopbits}")

# cmd="{space}"
# message = "{}".format(cmd)
# ser.write(message.encode())
# time.sleep(0.1)
# cmd="B" 
# message = "{}".format(cmd)
# ser.write(message.encode())
time.sleep(0.1)

ser.close()
if ser.isOpen():
    print("The serial port is enabled.")
else:
    print("The serial port is CLOSED.")