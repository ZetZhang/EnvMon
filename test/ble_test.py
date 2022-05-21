from bluepy.btle import Scanner, DefaultDelegate,Peripheral
import re

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        addr = "f6:84:22:8e:62:4a";
        #  if dev.addr == addr:
        print("Connected to BLE_Server")
        conn = Peripheral(addr)
        #  services = conn.getServices()
        #  for svc in services:
        #      print(svc.uuid)

        #  charac = conn.getCharacteristics(uuid="97e2a9d5-9dd3-4f64-8d74-d9e97773cbc9")
        #  info = conn.getDescriptors()
        #  resp = b'0x04'

        while True:
            try:
                #  response = charac[0].write(data, withResponse=True)
                #  print("descriptor: ", info[0])
                #  response = charac[0].read()
                #  print("response: ", response)
                #  charac[0].write(resp)
                
                services = conn.getServices()
                for svc in services:
                    print("[+] Service: {}".format(svc.uuid))
                    characteristics = svc.getCharacteristics()
                    for charac in characteristics:
                        print("    Characteristic: {}".format(charac.uuid))
                        print("         Properties: ",
                                charac.propertiesToString())
                    print("")
            except BaseException:
                print("No response!")
        print("disconnect")
        conn.disconnect()

scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(0, passive=True)
