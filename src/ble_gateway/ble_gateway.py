from bluepy.btle import Scanner, DefaultDelegate,Peripheral
import re
import struct
import time

global conn
ADDR = "f6:84:22:8e:62:4a";

CCCD_UUID = 0x2902
TEMPERATURE_SERVICE_UUID = "fe2c0000-0730-4a71-b132-4917c2bb832d"
HUMIDITY_SERVICE_UUID = "b1490000-5b47-44d0-88fd-5397b5511263"
PRESSURE_SERVICE_UUID = "41ec0000-6818-4108-80e8-82bd95504b7e"
SAMPLE_SERVICE_UUID = "855f0000-9f0f-49b7-88f7-d7f66145f461"
LIGHT_INTENSITY_SERVICE_UUID = "b4c20000-8684-4ba3-b784-e0b4a499a042"
CONTROL_SERVICE_UUID = "6c880000-6ca3-4775-9b56-c6ac4d0c1f72"
SENSOR_THRESHOLD_SERVICE_UUID = "75d40000-e036-4297-bea3-d9ea16d570e4"

TEMPERATURE_CHARACTERISTIC_UUID "fe2c0001-0730-4a71-b132-4917c2bb832d"
HUMIDITY_CHARACTERISTIC_UUID "b1490001-5b47-44d0-88fd-5397b5511263"
PRESSURE_CHARACTERISTIC_UUID "41ec0001-6818-4108-80e8-82bd95504b7e"
SAMPLE_CHARACTERISTIC_UUID "855f0001-9f0f-49b7-88f7-d7f66145f461"
LIGHT_INTENSITY_CHARACTERISTIC_UUID "b4c20001-8684-4ba3-b784-e0b4a499a042"
CONTROL_NOTICE_CHARACTERISTIC_UUID "6c880001-6ca3-4775-9b56-c6ac4d0c1f72"
SENSOR_THRESHOLD_CHARACTERISTIC_UUID "75d40001-e036-4297-bea3-d9ea16d570e4"

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device {}".format(dev.addr))
        elif isNewData:
            print("Recevied new data from {}".format(dev.addr))

class ReceiveDelegate(DefaultDelegate):
    def __init__(self, params):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        #  print("Notification from Handle: 0x{}".format(cHandle, "02X"))
        #  print(hexlify(data))
        print("[<--]", data)


def main_loop():
    #  scanner = Scanner().withDelegate(ScanDelegate())
    #  devices = scanner.scan(0, passive=True)

    #  print("Connected to BLE_Server")
    #  conn = Peripheral(ADDR)
    #  conn.setDelegate(ReceiveDelegate(conn))
    #  conn.setMTU(500)
    
    tempService = conn.getServiceByUUID(TEMPERATURE_SERVICE_UUID)
    humidService = conn.getServiceByUUID(HUMIDITY_SERVICE_UUID)
    pressService = conn.getServiceByUUID(PRESSURE_SERVICE_UUID)
    sampService = conn.getServiceByUUID(SAMPLE_SERVICE_UUID)
    lightService = conn.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
    controlService = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    thresholdService = conn.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

    tempCharacteristic = tempService.getCharacteristics()
    humidCharacteristic = humidService.getCharacteristics()
    pressCharacteristic = pressService.getCharacteristics()
    sampCharacteristic = sampService.getCharacteristics()
    lightIntensityCharacteristic = lightService.getCharacteristics();
    controlNoticeCharacteristic = controlService.getCharacteristics()
    sensorThresholdCharacteristic = thresholdService.getCharacteristics()

    #  controlS = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    #  control = controlS.getCharacteristics("6c880001-6ca3-4775-9b56-c6ac4d0c1f72")[0]
    #  control_cccd = control.getHandle()

    notify_on = b"\x03"
    CCCD_UUID = "6c880002-6ca3-4775-9b56-c6ac4d0c1f72"
    controlS = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    control = controlS.getCharacteristics("6c880001-6ca3-4775-9b56-c6ac4d0c1f72")[0]
    control.write(notify_on)
    #  ch_cccd = control.getDescriptors(forUUID=CCCD_UUID)[0]
    #  ch_cccd.write(notify_on)

    while True:
        for charac in tempCharacteristic:
            [temperature] = struct.unpack('f', charac.read())
            print("temperature data:", round(temperature, 2), "°C")

        for charac in humidCharacteristic:
            [humidity] = struct.unpack('f', charac.read())
            print("humidity data:", round(humidity, 2), "%")

        for charac in pressCharacteristic:
            [pressure] = struct.unpack('f', charac.read())
            print("pressure data:", round(pressure, 2), "kPa")

        for charac in sampCharacteristic:
            sample = struct.unpack('b', charac.read())
            print("sample data:", sample, "dB")

        for charac in lightIntensityCharacteristic:
            [lightIntensity] = struct.unpack('b', charac.read())
            print("light intensity data:", lightIntensity, "steps")

        for charac in controlNoticeCharacteristic:
            #  charac.write(b'\x2a');
            print("control data:", charac.read())

        for charac in sensorThresholdCharacteristic:
            thresholdList = list(charac.read())
            print("threshold data list: ", thresholdList)

        print("------------------------------------------")

        time.sleep(1)

def noticeEnable(conn):
    print("Connected to BLE_Server")
    conn.setDelegate(ReceiveDelegate(conn))
    conn.setMTU(500)

    tempService = conn.getServiceByUUID(TEMPERATURE_SERVICE_UUID)
    humidService = conn.getServiceByUUID(HUMIDITY_SERVICE_UUID)
    pressService = conn.getServiceByUUID(PRESSURE_SERVICE_UUID)
    sampService = conn.getServiceByUUID(SAMPLE_SERVICE_UUID)
    lightService = conn.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
    controlService = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    thresholdService = conn.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

    tempCharacteristic = tempService.getCharacteristics()
    humidCharacteristic = humidService.getCharacteristics()
    pressCharacteristic = pressService.getCharacteristics()
    sampCharacteristic = sampService.getCharacteristics()
    lightIntensityCharacteristic = lightService.getCharacteristics();
    controlNoticeCharacteristic = controlService.getCharacteristics()
    sensorThresholdCharacteristic = thresholdService.getCharacteristics()

    tempHandle = tempCharacteristic[0].getHandle()
    humidHandle = humidCharacteristic[0].getHandle()
    pressHandle= pressCharacteristic[0].getHandle()
    samphandle = sampCharacteristic[0].getHandle()
    lightIntensityHandle = lightIntensityCharacteristic[0].getHandle()
    controlHandle = controlNoticeCharacteristic[0].getHandle()
    sensorHandle = sensorThresholdCharacteristic[0].getHandle()

    #  temp_cccd = 0
    #  for descriptor in tempCharacteristic[0].getDescriptors(tempHandle, tempService.hndEnd):
    #      if (descriptor == 0x2902):
    #          print(f'Client Characteristic Configuration found at handle 0x{format(descriptor.handle,"02X")}')
    #          temp_cccd = descriptor.handle
    #  if temp_cccd != 0:
    #      conn.writeCharacteristic(temp_cccd, bytes([1, 0]))

    notify_on = b"\0x1\x00"
    CCCD_UUID = 0x2902
    controlS = conn.getServiceByUUID(TEMPERATURE_SERVICE_UUID)
    control = controlS.getCharacteristics("fe2c0001-0730-4a71-b132-4917c2bb832d")[0]
    ch_cccd = control.getDescriptors(forUUID=CCCD_UUID)[0]
    ch_cccd.write(notify_on)
    #  control_cccd = control.getHandle()

def notification(conn):
    while True:
        if conn.waitForNotifications(1.0):
            # handleNotification was called
            continue
        print("Waiting notification")
    #      conn.disconnect()
    #      break


if __name__ == '__main__':
    conn = Peripheral(ADDR)
    #  main_loop()
    noticeEnable(conn)
    notification(conn)
