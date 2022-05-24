import queue
import json
import struct

class DataQueue:
    def __init__(self, max):
        self.queueData = queue.Queue(max)

    def pushData(self, key, value):
        #  self.queueData.put_nowait({key: value})
        self.queueData.put_nowait((key, value))

    def popData(self):
        #  return self.queueData.get_nowait()[key]
        return self.queueData.get_nowait()

    def takeData(self):
        return self.popData()[1]

    def takeKey(self):
        return self.popData()[0]

    def waitAllDone(self):
        return self.queueData.join()

class DataBuffer:
    def __init__(self):
        self.controlFlag = False
        self.thresholdFlag = False
        self.temperature = ""
        self.humidity = ""
        self.pressure = ""
        self.sample = ""
        self.lightIntensity = ""
        self.controlNotice = ""
        self.threshold = ""

    def isSensorDataReady(self):
        print("{}, {}, {}, {}, {}".format(self.temperature, self.humidity, self.pressure, self.sample, self.lightIntensity))
        return self.temperature != "" and self.humidity != "" and self.pressure != "" and self.sample != "" and self.lightIntensity

    def sensorDataReset(self):
        self.temperature = ""
        self.humidity = ""
        self.pressure = ""
        self.sample = ""
        self.lightIntensity = ""

    def controlDataReset(self):
        self.controlNotice = ""
        self.controlFlag = False

    def thresdDataReset(self):
        self.threshold = ""
        self.thresholdFlag = False

    def setTemperature(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.temperature = str(res)

    def setHumidity(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.humidity = str(res)

    def setPressure(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.pressure = str(res)

    def setSample(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('b', value)
            self.sample = str(val)

    def setLightIntensity(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('b', value)
            self.lightIntensity = str(val)

    def setControlNotice(self, value):
        self.controlFlag = True
        if isinstance(value, bytes):
            self.controlNotice = str(value)

    def setThreshold(self, value):
        self.thresholdFlag = True
        if isinstance(value, bytes):
            self.threshold = str(value)

    def _sensorJsonData(self):
        if self.isSensorDataReady():
            data = [{'temperature': self.temperature},
                    {'humidity': self.humidity},
                    {'pressure': self.pressure},
                    {'sample': self.sample},
                    {'lightintensity': self.lightIntensity}
                    ]

            self.sensorDataReset()

            return data
        return None

    def getSensorJsonData(self):
        sensorData = self._sensorJsonData()

        data = {'identity': 2,
                'sensorInfo': sensorData}

        jsonData = json.dumps(data)
        return jsonData

    def _controlJsonData(self):
        if self.controlNotice == "":
            return None

        data = {'controlNotice': self.controlNotice}

        self.controlDataReset()

        self.controlNotice = ""
        return data

    def getControlJsonData(self):
        controlData = self._controlJsonData()

        data = {'identity': 3,
                'controlNotice': controlData}

        jsonData = json.dumps(data)
        return jsonData

    def _thresholdJsonData(self):
        if self.threshold == "":
            return None

        data = {'thresholdList': self.threshold}

        self.thresdDataReset()

        self.threshold = ""
        return data

    def getThresholdJsonData(self):
        thresholdList = self._thresholdJsonData()

        data = {'identity': 4,
                'thresholdList': thresholdList}

        jsonData = json.dumps(data)
        return jsonData

    def getJsonData(self):
        controlData = self._controlJsonData()
        thresholdData = self._thresholdJsonData()
        sensorData = self._sensorJsonData()

        if (controlData is None and thresholdData is None and sensorData is None):
            return None

        data = {'identity': 1,
                'control': controlData,
                'threshold': thresholdData,
                'sensor': sensorData}
        jsonData = json.dumps(data)
        return jsonData
