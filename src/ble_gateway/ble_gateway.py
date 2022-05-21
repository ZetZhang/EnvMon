from bluepy.btle import Scanner, DefaultDelegate,Peripheral
import re
import struct
import time

global conn
ADDR = "f6:84:22:8e:62:4a";

TEMPERATURE_SERVICE_UUID = "fe2c0000-0730-4a71-b132-4917c2bb832d"
HUMIDITY_SERVICE_UUID = "b1490000-5b47-44d0-88fd-5397b5511263"
PRESSURE_SERVICE_UUID = "41ec0000-6818-4108-80e8-82bd95504b7e"
SAMPLE_SERVICE_UUID = "855f0000-9f0f-49b7-88f7-d7f66145f461"
LIGHT_INTENSITY_SERVICE_UUID = "b4c20000-8684-4ba3-b784-e0b4a499a042"
CONTROL_SERVICE_UUID = "6c880000-6ca3-4775-9b56-c6ac4d0c1f72"
SENSOR_THRESHOLD_SERVICE_UUID = "75d40000-e036-4297-bea3-d9ea16d570e4"

#  LED_ON = bytes([0x03])
#  LED_OFF = bytes([0x02])
#  AUTO_FLIP = bytes([0x00])
#  NOTICE_ON = bytes([0x04])

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device {}".format(dev.addr))
        elif isNewData:
            print("Recevied new data from {}".format(dev.addr))

class ReceiveDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        #  print("Notification from Handle: 0x{}".format(cHandle, "02X"))
        #  print(hexlify(data))
        print("[<--]", data)


def main_loop():
    #  scanner = Scanner().withDelegate(ScanDelegate())
    #  devices = scanner.scan(0, passive=True)

    print("Connected to BLE_Server")
    conn = Peripheral(ADDR)
    conn.withDelegate(ReceiveDelegate())
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

    controlS = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    control = controlS.getCharacteristics("6c880001-6ca3-4775-9b56-c6ac4d0c1f72")[0]
    control_cccd = control.getHandle()

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
            charac.write(b'\x2a');
            print("control data:", charac.read())

        for charac in sensorThresholdCharacteristic:
            thresholdList = list(charac.read())
            print("threshold data list: ", thresholdList)

        if conn.waitForNotifications(1.0):
            continue

        print("------------------------------------------")

        time.sleep(1)

    #  while True:
    #      if conn.waitForNotifications(1.0):
    #          # handleNotification was called
    #          continue
    #      print("Waiting notification")
    #      conn.disconnect()
    #      break

if __name__ == '__main__':
    main_loop()
