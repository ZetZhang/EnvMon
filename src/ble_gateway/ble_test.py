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
