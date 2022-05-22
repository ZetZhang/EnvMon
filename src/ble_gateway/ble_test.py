from bluepy import btle

class MyDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print(data)

# Initialisation  -------
ADDRESS = "f6:84:22:8e:62:4a"

CHARACTERISTIC_UUID = "6c880001-6ca3-4775-9b56-c6ac4d0c1f72"
SERVICE_UUID = "6c880000-6ca3-4775-9b56-c6ac4d0c1f72"

p = btle.Peripheral(ADDRESS)
p.setDelegate(MyDelegate())

#  notif_on = b"\0x1\x00"
#  notif_on = b"\0x0\x01"

CCCD_UUID = 0x2902
svc = p.getServiceByUUID(SERVICE_UUID)
ch = svc.getCharacteristics(CHARACTERISTIC_UUID)[0]
ch_cccd = ch.getDescriptors(forUUID=CCCD_UUID)[0]
response1 = ch_cccd.write(bytes([1, 0]))
print(response1)
response2 = ch.write(bytes([3]), withResponse=True)
print(response2)

while True:
    if p.waitForNotifications(1.0):
        print("notification received")
        # handleNotification() was called
        continue

    print("Waiting...")

#  def main_loop(conn):
#      print("Connected to BLE_Server")
#      conn.setMTU(500)

#      humidService = conn.getServiceByUUID(HUMIDITY_SERVICE_UUID)
#      pressService = conn.getServiceByUUID(PRESSURE_SERVICE_UUID)
#      sampService = conn.getServiceByUUID(SAMPLE_SERVICE_UUID)
#      lightService = conn.getServiceByUUID(LIGHT_INTENSITY_SERVICE_UUID)
#      controlService = conn.getServiceByUUID(CONTROL_SERVICE_UUID)
#      thresholdService = conn.getServiceByUUID(SENSOR_THRESHOLD_SERVICE_UUID)

#      humidCharacteristic = humidService.getCharacteristics(HUMIDITY_CHARACTERISTIC_UUID)
#      pressCharacteristic = pressService.getCharacteristics(PRESSURE_CHARACTERISTIC_UUID)
#      sampCharacteristic = sampService.getCharacteristics(SAMPLE_CHARACTERISTIC_UUID)
#      lightIntensityCharacteristic = lightService.getCharacteristics(LIGHT_INTENSITY_CHARACTERISTIC_UUID)
#      controlNoticeCharacteristic = controlService.getCharacteristics(CONTROL_NOTICE_CHARACTERISTIC_UUID)
#      sensorThresholdCharacteristic = thresholdService.getCharacteristics(SENSOR_THRESHOLD_CHARACTERISTIC_UUID)

#      while True:
#          for charac in humidCharacteristic:
#              [humidity] = struct.unpack('f', charac.read())
#              print("humidity data:", round(humidity, 2), "%")

#          for charac in pressCharacteristic:
#              [pressure] = struct.unpack('f', charac.read())
#              print("pressure data:", round(pressure, 2), "kPa")

#          for charac in sampCharacteristic:
#              sample = struct.unpack('b', charac.read())
#              print("sample data:", sample, "dB")

#          for charac in lightIntensityCharacteristic:
#              [lightIntensity] = struct.unpack('b', charac.read())
#              print("light intensity data:", lightIntensity, "steps")

#          for charac in controlNoticeCharacteristic:
#              #  charac.write(b'\x2a');
#              print("control data:", charac.read())

#          for charac in sensorThresholdCharacteristic:
#              thresholdList = list(charac.read())
#              print("threshold data list: ", thresholdList)

#          print("------------------------------------------")

#          time.sleep(1)
