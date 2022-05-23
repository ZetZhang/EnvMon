from bluepy.btle import DefaultDelegate
import BleSense
import threading
import struct

from tornado.ioloop import IOLoop, PeriodicCallback
from tornado import gen
from tornado.websocket import websocket_connect

class ReceiveDelegate(DefaultDelegate):
    def __init__(self, peripheral):
        DefaultDelegate.__init__(self)
        self.peripheral = peripheral

    def handleNotification(self, cHandle, data):
        #  print("[<--]", data)
        #  ret = {
        #          self.peripheral.getTemperatureHandle(): lambda data: print("yes"),
        #  } [cHandle](data)
        if cHandle == self.peripheral.getTemperatureHandle():
            [temperature] = struct.unpack('f', data)
            print("temperature data:", round(temperature, 2), "°C")
        elif cHandle == self.peripheral.getHumidityHandle():
            [humidity] = struct.unpack('f', data)
            print("humidity data:", round(humidity, 2), "%")
        elif cHandle == self.peripheral.getPressureHandle():
            [pressure] = struct.unpack('f', data)
            print("pressure data:", round(pressure, 2), "kPA")
        elif cHandle == self.peripheral.getSampleHandle():
            [sample] = struct.unpack('b', data)
            print("sample data:", sample, "dB")
        elif cHandle == self.peripheral.getLightIntensityHandle():
            [lightIntensity] = struct.unpack('b', data)
            print("light intensity data:", lightIntensity, "steps")
        elif cHandle == self.peripheral.getControlNoticeHandle():
            [control] = struct.unpack('b', data)
            lightStr = "on" if control & 1 else "off"
            lightControlStr = "on" if control & 2 else "off"
            smartReminderStr = "on" if control & 4 else "off"
            print("control data: [ light:", lightStr, ", light control:", lightControlStr, ", smart reminder:", smartReminderStr, "]")
        elif cHandle == self.peripheral.getThresholdHandle():
            thresholdList = list(data)
            print("threshold data list:", thresholdList)

class wsConnect(object):
    def __init__(self, url, timeout):
        self.count = 0
        self.url = url
        self.timeout = timeout
        self.ioloop = IOLoop.instance()
        self.ws = None
        self.connect()
        PeriodicCallback(self.keep_alive, 20000).start()
        self.ioloop.start()

    @gen.coroutine
    def connect(self):
        #  print("trying to connect")
        try:
            self.ws = yield websocket_connect(self.url)
        except:
            print("connection error")
        else:
            print("conncted")
            self.run()

    @gen.coroutine
    def run(self):
        while True:
            self.count += 1
            yield self.ws.write_message('{}'.format(self.count))
            msg = yield self.ws.read_message()
            if msg is None:
                print("connection closed")
                self.ws = None
                break
            print('message: {}'.format(msg))
            yield gen.sleep(2)

    def keep_alive(self):
        if self.ws is None:
            self.connect()
        else:
            self.ws.write_message("keep alive")

def sensor_acquisition_service(name, peripheral):
    print("{} threading running.".format(name))
    peripheral = BleSense.PeripheralService()
    peripheral.notifyHandler(ReceiveDelegate)
    peripheral.notificationEnable()
    peripheral.loop()

if __name__ == '__main__':
    try:
        pService = 0
        sThread = threading.Thread(target=sensor_acquisition_service, args=('mas', pService))

        sThread.start()

        print("connting...")
        wsConnect("ws://localhost:12340/echo", 5)

    except:
        print("start thread errro...")

