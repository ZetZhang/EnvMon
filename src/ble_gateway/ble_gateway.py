from bluepy.btle import DefaultDelegate
import BleSense
import threading
import struct
import json

from tornado.ioloop import IOLoop, PeriodicCallback
from tornado import gen
from tornado.websocket import websocket_connect

import BufferCoordinator

#  dataQueue = BufferCoordinator.DataQueue(10)
dataBuffer = BufferCoordinator.DataBuffer()

class ReceiveDelegate(DefaultDelegate):
    def __init__(self, peripheral):
        DefaultDelegate.__init__(self)
        self.peripheral = peripheral

    def handleNotification(self, cHandle, data):
        #  print("[<--]", data)
        #  ret = {
        #          self.peripheral.getTemperatureHandle(): lambda data: print("yes"),
        #  } [cHandle](data)
        if cHandle is self.peripheral.getTemperatureHandle():
            dataBuffer.setTemperature(data)
            [temperature] = struct.unpack('f', data)
            print("temperature data:", round(temperature, 2), "°C")
        elif cHandle is self.peripheral.getHumidityHandle():
            dataBuffer.setHumidity(data)
            [humidity] = struct.unpack('f', data)
            print("humidity data:", round(humidity, 2), "%")
        elif cHandle is self.peripheral.getPressureHandle():
            dataBuffer.setPressure(data)
            [pressure] = struct.unpack('f', data)
            print("pressure data:", round(pressure, 2), "kPA")
        elif cHandle is self.peripheral.getSampleHandle():
            dataBuffer.setSample(data)
            [sample] = struct.unpack('b', data)
            print("sample data:", sample, "dB")
        elif cHandle is self.peripheral.getLightIntensityHandle():
            dataBuffer.setLightIntensity(data)
            [lightIntensity] = struct.unpack('b', data)
            print("light intensity data:", lightIntensity, "steps")
        elif cHandle is self.peripheral.getControlNoticeHandle():
            dataBuffer.setControlNotice(data)
            [control] = struct.unpack('b', data)
            lightStr = "on" if control & 1 else "off"
            lightControlStr = "on" if control & 2 else "off"
            smartReminderStr = "on" if control & 4 else "off"
            print("control data: [ light:", lightStr, ", light control:", lightControlStr, ", smart reminder:", smartReminderStr, "]")
        elif cHandle is self.peripheral.getThresholdHandle():
            dataBuffer.setThreshold(data)
            thresholdList = list(data)
            print("threshold data list:", thresholdList)

class wsConnect():
    def __init__(self, url, timeout):
        self.url = url
        self.timeout = timeout
        self.ioloop = IOLoop.instance()
        self.ws = None
        self.connect()
        PeriodicCallback(self.keep_alive, 20000).start()
        self.ioloop.start()

    @gen.coroutine
    def connect(self):
        try:
            self.ws = yield websocket_connect(self.url, on_message_callback = self.on_message)
        except:
            print("connection error")
        else:
            print("conncted")
            self.run()

    @gen.coroutine
    def run(self):
        while True:
            print("-------------------------------------------------------")
            jsonData = dataBuffer.getJsonData()
            if jsonData is not None:
                print(jsonData)
                yield self.ws.write_message(jsonData)
            yield gen.sleep(1)

    def on_message(self, msg):
        message = json.loads(msg)
        identity = message["identity"]
        if identity == 0:
            print('get: {}'.format(message))
        elif identity == 3:
            print('control: {}'.format(message))
        elif identity == 4:
            print('threshold: {}'.format(message))
        else:
            print("step.....................")

    def keep_alive(self):
        if self.ws is None:
            self.connect()
        else:
            self.ws.write_message("keep alive")

def sensor_acquisition_service():
    peripheral = BleSense.PeripheralService()
    peripheral.notifyHandler(ReceiveDelegate)
    peripheral.notificationEnable()
    peripheral.loop()

#  def recvControl():
#      conn = websocket_connect("ws://localhost:12340/recvControl")
#      while True:
#          msg = conn.read_message()
#          if msg is None:
#              break;
#          print(msg)

if __name__ == '__main__':
    try:
        sThread = threading.Thread(target=sensor_acquisition_service)

        print("sensor acquisition threading running...")
        sThread.start()

        print("connting...")
        wsConnect("ws://localhost:12340/periodicDataUpload?who=user", 5)

    except:
        print("start thread errro...")

