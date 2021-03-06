from bluepy.btle import Scanner, DefaultDelegate,Peripheral

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

class PeripheralService:
    device_addr = "f6:84:22:8e:62:4a"

    def __init__(self):
        self.peripheral = Peripheral(self.device_addr)
        self.peripheral.setMTU(300)

        self.tempService = self.peripheral.getServiceByUUID(TEMPERATURE_SERVICE_UUID) 
        self.humidService = self.peripheral.getServiceByUUID(HUMIDITY_SERVICE_UUID)
        self.pressService = self.peripheral.getServiceByUUID(PRESSURE_SERVICE_UUID)
        self.sampService = self.peripheral.getServiceByUUID(SAMPLE_SERVICE_UUID)
        self.lightService = self.peripheral.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
        self.controlService = self.peripheral.getServiceByUUID(CONTROL_SERVICE_UUID)
        self.thresholdService = self.peripheral.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

        self.tempCharacteristic = self.tempService.getCharacteristics(TEMPERATURE_CHARACTERISTIC_UUID)[0]
        self.humidCharacteristic = self.humidService.getCharacteristics(HUMIDITY_CHARACTERISTIC_UUID)[0]
        self.pressCharacteristic = self.pressService.getCharacteristics(PRESSURE_CHARACTERISTIC_UUID)[0]
        self.sampCharacteristic = self.sampService.getCharacteristics(SAMPLE_CHARACTERISTIC_UUID)[0]
        self.lightIntensityCharacteristic = self.lightService.getCharacteristics(LIGHT_INTENSITY_CHARACTERISTIC_UUID)[0]
        self.controlNoticeCharacteristic = self.controlService.getCharacteristics(CONTROL_NOTICE_CHARACTERISTIC_UUID)[0]
        self.sensorThresholdCharacteristic = self.thresholdService.getCharacteristics(SENSOR_THRESHOLD_CHARACTERISTIC_UUID)[0]

    def getTemperatureHandle(self):
        return self.tempCharacteristic.getHandle()

    def getHumidityHandle(self):
        return self.humidCharacteristic.getHandle()

    def getPressureHandle(self):
        return self.pressCharacteristic.getHandle()

    def getSampleHandle(self):
        return self.sampCharacteristic.getHandle()

    def getLightIntensityHandle(self):
        return self.lightIntensityCharacteristic.getHandle()

    def getControlNoticeHandle(self):
        return self.controlNoticeCharacteristic.getHandle()

    def getThresholdHandle(self):
        return self.sensorThresholdCharacteristic.getHandle()

    def disconnect(self):
        self.disconnect()

    def notifyHandler(self, cb):
        self.peripheral.setDelegate(cb(self))
        self.cb_backup = cb

    def sendControlTo(self, data):
        self.controlNoticeCharacteristic.write(data.to_bytes(2, 'little'), withResponse=True)

    def sendThresholdTo(self, info):
        self.sensorThresholdCharacteristic.write(bytes(info), withResponse=True)

    def isConnected(self):
        try:
            return self.peripheral.getState() == 'conn'
        except:
            return False

    def connect(self):
        if (self.isConnected() is False):
            print("reconnecting")
            self.peripheral.connect(self.device_addr)
            self.peripheral.setDelegate(self.cb_backup(self))
            self.notificationEnable()
            self.indicationEnable()
        return True

    def notificationEnable(self):
        self.tempCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.humidCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.pressCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.sampCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.lightIntensityCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.controlNoticeCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))
        self.sensorThresholdCharacteristic.getDescriptors(forUUID=CCCD_UUID)[0].write(bytes([1, 0]))

    def indicationEnable(self):
        pass
