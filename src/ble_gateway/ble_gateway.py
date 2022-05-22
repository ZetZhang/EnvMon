from bluepy.btle import Scanner, DefaultDelegate,Peripheral
import struct
import time
import threading

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

TEMPERATURE_CHARACTERISTIC_UUID = "fe2c0001-0730-4a71-b132-4917c2bb832d"
HUMIDITY_CHARACTERISTIC_UUID = "b1490001-5b47-44d0-88fd-5397b5511263"
PRESSURE_CHARACTERISTIC_UUID = "41ec0001-6818-4108-80e8-82bd95504b7e"
SAMPLE_CHARACTERISTIC_UUID = "855f0001-9f0f-49b7-88f7-d7f66145f461"
LIGHT_INTENSITY_CHARACTERISTIC_UUID = "b4c20001-8684-4ba3-b784-e0b4a499a042"
CONTROL_NOTICE_CHARACTERISTIC_UUID = "6c880001-6ca3-4775-9b56-c6ac4d0c1f72"
SENSOR_THRESHOLD_CHARACTERISTIC_UUID = "75d40001-e036-4297-bea3-d9ea16d570e4"


class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device {}".format(dev.addr))
        elif isNewData:
            print("Recevied new data from {}".format(dev.addr))


def main_loop(conn):
    print("Connected to BLE_Server")
    conn.setMTU(500)

    humidService = conn.getServiceByUUID(HUMIDITY_SERVICE_UUID)
    pressService = conn.getServiceByUUID(PRESSURE_SERVICE_UUID)
    sampService = conn.getServiceByUUID(SAMPLE_SERVICE_UUID)
    lightService = conn.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
    controlService = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
    thresholdService = conn.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

    humidCharacteristic = humidService.getCharacteristics(HUMIDITY_CHARACTERISTIC_UUID)
    pressCharacteristic = pressService.getCharacteristics(PRESSURE_CHARACTERISTIC_UUID)
    sampCharacteristic = sampService.getCharacteristics(SAMPLE_CHARACTERISTIC_UUID)
    lightIntensityCharacteristic = lightService.getCharacteristics(LIGHT_INTENSITY_CHARACTERISTIC_UUID)
    controlNoticeCharacteristic = controlService.getCharacteristics(CONTROL_NOTICE_CHARACTERISTIC_UUID)
    sensorThresholdCharacteristic = thresholdService.getCharacteristics(SENSOR_THRESHOLD_CHARACTERISTIC_UUID)

    while True:
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

class ReceiveDelegate(DefaultDelegate):
    def __init__(self, peripheral):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        #  print("Notification from Handle: 0x{}".format(cHandle, "02X"))
        #  print(hexlify(data))
        #  cHandle.getHandle().write(bytes(3))
        print("[<--]", data)

class PeripheralService:
    def __init__(self, addr):
        self.conn = Peripheral(addr)
        self.conn.setMTU(300)

        self.tempService = self.conn.getServiceByUUID(TEMPERATURE_SERVICE_UUID)
        self.humidService = self.conn.getServiceByUUID(HUMIDITY_SERVICE_UUID)
        self.pressService = self.conn.getServiceByUUID(PRESSURE_SERVICE_UUID)
        self.sampService = self.conn.getServiceByUUID(SAMPLE_SERVICE_UUID)
        self.lightService = self.conn.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
        self.controlService = self.conn.getServiceByUUID(CONTROL_SERVICE_UUID)
        self.thresholdService = self.conn.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

        self.tempCharacteristic = self.tempService.getCharacteristics(TEMPERATURE_CHARACTERISTIC_UUID)[0]
        self.humidCharacteristic = self.humidService.getCharacteristics(HUMIDITY_CHARACTERISTIC_UUID)[0]
        self.pressCharacteristic = self.pressService.getCharacteristics(PRESSURE_CHARACTERISTIC_UUID)[0]
        self.sampCharacteristic = self.sampService.getCharacteristics(SAMPLE_CHARACTERISTIC_UUID)[0]
        self.lightIntensityCharacteristic = self.lightService.getCharacteristics(LIGHT_INTENSITY_CHARACTERISTIC_UUID)[0]
        self.controlNoticeCharacteristic = self.controlService.getCharacteristics(CONTROL_NOTICE_CHARACTERISTIC_UUID)[0]
        self.sensorThresholdCharacteristic = self.thresholdService.getCharacteristics(SENSOR_THRESHOLD_CHARACTERISTIC_UUID)[0]

    def disconnect(self):
        self.disconnect()

    def notifyHandler(self, cb):
        self.conn.setDelegate(cb(self.conn))

        self.tempCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.humidCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.pressCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.sampCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.lightIntensityCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.controlNoticeCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))

    def sendControlInfo(self, bMes):
        response = self.controlNoticeCharacteristic.write(bytes([bMes]), withResponse=True)
        print("message sending...")
        print(response)

    def loop(self):
        while True:
            if self.conn.waitForNotifications(1.0):
                continue

# https://github.com/IanHarvey/bluepy/issues/360
def sensor_acquisition_service(name, peripheral):
    print("{} threading running.".format(name))
    peripheral.notifyHandler(ReceiveDelegate)
    peripheral.loop()

def message_sending_service(name, peripheral):
    print("{} threading running.".format(name))
    while True:
        peripheral.sendControlInfo(3)
        time.sleep(3)

if __name__ == '__main__':
    pService = PeripheralService(ADDR)
    try:
        mThread = threading.Thread(target=message_sending_service, args=('mss', pService))
        sThread = threading.Thread(target=sensor_acquisition_service, args=('mas', pService))

        mThread.start()
        sThread.start()

    except:
        print("start thread errro...")

