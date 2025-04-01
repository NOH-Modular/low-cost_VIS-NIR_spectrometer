#########################################################################################################################################
#########################################################################################################################################
### Library Imports

import machine
import time
from machine import I2C, Pin

#########################################################################################################################################
#########################################################################################################################################
### AS7341 Library

#AS7341 address
AS7341_ADDR = const(0x39)

#AS7341 registers
#ADC Channels
AS_CH0_DATA_L = const(0x95)
AS_CH1_DATA_L = const(0x97)
AS_CH2_DATA_L = const(0x99)
AS_CH3_DATA_L = const(0x9B)
AS_CH4_DATA_L = const(0x9D)
AS_CH5_DATA_L = const(0x9F)

#CFG Registers
AS_CFG0 = const(0xA9)
AS_CFG1 = const(0xAA)
AS_CFG3 = const(0xAC)
AS_CFG6 = const(0xAF)
AS_CFG8 = const(0xB1)
AS_CFG9 = const(0xB2)
AS_CFG10 = const(0xB3)
AS_CFG12 = const(0xB5)

#Status Registers
AS_STATUS2 = const(0xA3)
AS_STATUS3 = const(0xA4)
AS_STATUS5 = const(0xA6)
AS_STATUS6 = const(0xA7)

#misc
AS_ASTATUS = const(0x60)
AS_ENABLE = const(0x80)
AS_ATIME = const(0x81)
AS_ASTEP_L = const(0xCA)
AS_ASTEP_H = const(0xCB)

#SMUX Configurations
SMUX0 = bytearray(20)
SMUX1 = bytearray(20)
SMUX0 = [0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x10, 0x03, 0x50, 0x10, 0x03, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x50, 0x00, 0x06]
SMUX1 = [0x30, 0x01, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x20, 0x04, 0x00, 0x30, 0x01, 0x50, 0x00, 0x06]

def setATIME(atime):
    #For peripheral at address given, write to register AS_ATIME the argument atime which is eight bits
    i2c.writeto_mem(AS7341_ADDR, AS_ATIME, bytes([atime]))
    
def setASTEP(astep_l, astep_h):
    #write to two consecutive registers
    i2c.writeto_mem(AS7341_ADDR, AS_ASTEP_L, bytes([astep_l]))
    i2c.writeto_mem(AS7341_ADDR, AS_ASTEP_H, bytes([astep_h]))
    
def setGAIN(gain):
    i2c.writeto_mem(AS7341_ADDR, AS_CFG1, bytes([gain]))
    
def SpEn(isenable):
    temp = i2c.readfrom_mem(AS7341_ADDR, AS_ENABLE, 1)[0]
    temp = temp & 0xFD
    if isenable:
        temp = temp | 0x02
    else:
        temp = temp & 0xFD
    i2c.writeto_mem(AS7341_ADDR, AS_ENABLE, bytes([temp]))
    
def ReadValues(dataset):
    global SMUX0, SMUX1
    #set PON, read ENABLE then clear bit 0
    temp1 = i2c.readfrom_mem(AS7341_ADDR, AS_ENABLE, 1)[0]
    temp1 = temp1 & 0xFE
    temp1 = temp1 | 0x01
    i2c.writeto_mem(AS7341_ADDR, AS_ENABLE, bytes([temp1]))
    
    #disable spectral processing
    SpEn(False)
    
    #set SMUX to take config from RAM (CFG6)
    i2c.writeto_mem(AS7341_ADDR, AS_CFG6, bytes([0x10]))
    
    #write SMUX config to first 20 registers
    if(dataset == 0):
        i2c.writeto_mem(AS7341_ADDR, 0x01, bytearray(SMUX0))
    else:
        i2c.writeto_mem(AS7341_ADDR, 0x01, bytearray(SMUX1))
        
    #enable SMUXEN bit (ENABLE)
    temp = i2c.readfrom_mem(AS7341_ADDR, AS_ENABLE, 1)[0]
    temp = temp & 0xEF
    temp = temp | 0x10
    i2c.writeto_mem(AS7341_ADDR, AS_ENABLE, bytes([temp]))
        
    #wait until SMUX is enabled (set bit in previous step resets to 0)
    
    
    #enable spectral processing (SP_EN)
    SpEn(True)
    
    #wait for data to be ready
    
    #read dataset
    if dataset:
        print("channel 1 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH0_DATA_L, 2))
        print("channel 2 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH1_DATA_L, 2))
        print("channel 3 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH2_DATA_L, 2))
        print("channel 4 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH3_DATA_L, 2))
        print("clear - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH4_DATA_L, 2))
        print("NIR - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH5_DATA_L, 2))
    else:
        print("channel 5 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH0_DATA_L, 2))
        print("channel 6 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH1_DATA_L, 2))
        print("channel 7 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH2_DATA_L, 2))
        print("channel 8 - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH3_DATA_L, 2))
        print("clear - ",(i2c.readfrom_mem(AS7341_ADDR, AS_CH4_DATA_L, 2)))
        print("NIR - ",i2c.readfrom_mem(AS7341_ADDR, AS_CH5_DATA_L, 2))
    

#########################################################################################################################################
#########################################################################################################################################
### Set-Up and Main Loop
    
##SDA = Pin(0, Pin.OUT)
##SCL = Pin(1, Pin.OUT)
i2c = I2C(0,100000)
##Center = Pin(0, Pin.OUT)
##Center.value(0)
print(i2c.scan())


while True:
    setATIME(0x64)
    #need to check if bytes are the correct way around
    setASTEP(0x03, 0xE7)
    setGAIN(0x09)
    ReadValues(0)
    time.sleep(50)