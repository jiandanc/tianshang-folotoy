#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""抓取 ESP32-C3 启动日志:复位后读取串口输出。"""
import sys
import time
import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM3"
SECONDS = float(sys.argv[2]) if len(sys.argv) > 2 else 12.0

ser = serial.Serial(PORT, 115200, timeout=0.2)

# 通过 RTS 拉低复位芯片(ESP32-C3 USB-Serial/JTAG 支持)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.15)
ser.setRTS(False)
time.sleep(0.1)

deadline = time.time() + SECONDS
buf = bytearray()
while time.time() < deadline:
    chunk = ser.read(4096)
    if chunk:
        buf.extend(chunk)

ser.close()
sys.stdout.write(buf.decode("utf-8", errors="replace"))
