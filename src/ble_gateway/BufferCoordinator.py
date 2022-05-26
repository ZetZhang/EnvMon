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
        self.temperature = 0.0
        self.humidity = 0.0
        self.pressure = 0.0
        self.sample = 0
        self.lightIntensity = 0
        self.controlNotice = -1
        self.threshold = []

    def isSensorDataReady(self):
        return self.temperature != 0.0 and self.humidity != 0.0 and self.pressure != 0.0 and self.sample != 0 and self.lightIntensity != 0

    def sensorDataReset(self):
        self.temperature = 0.0
        self.humidity = 0.0
        self.pressure = 0.0
        self.sample = 0
        self.lightIntensity = 0

    def controlDataReset(self):
        self.controlNotice = -1
        self.controlFlag = False

    def thresdDataReset(self):
        self.threshold = []
        self.thresholdFlag = False

    def setTemperature(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.temperature = res
        else:
            self.temperature = value

    def setHumidity(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.humidity = res
        else:
            self.humidity = value

    def setPressure(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.pressure = res
        else:
            self.pressure = value

    def setSample(self, value):
        if isinstance(value, bytes):
            val = int().from_bytes(value, byteorder='big', signed=True)
            self.sample = val
        else:
            self.sample = value

    def setLightIntensity(self, value):
        if isinstance(value, bytes):
            val = int().from_bytes(value, byteorder='big', signed=True)
            self.lightIntensity = val
        else:
            self.lightIntensity = value

    def setControlNotice(self, value):
        if isinstance(value, bytes):
            self.controlFlag = True
            val = int().from_bytes(value, byteorder='big', signed=True)
            self.controlNotice = val
        else:
            self.controlFlag = True
            self.controlNotice = value

    def setThreshold(self, value):
        self.thresholdFlag = True
        self.threshold = value

    def _sensorJsonData(self):
        if self.isSensorDataReady():
            data = {'temperature': self.temperature,
                    'humidity': self.humidity,
                    'pressure': self.pressure,
                    'sample': self.sample,
                    'lightintensity': self.lightIntensity}

            return data
        return None

    def getSensorJsonData(self):
        sensorData = self._sensorJsonData()

        data = {'identity': 2,
                'sensorInfo': sensorData}

        self.sensorDataReset()
        jsonData = json.dumps(data)
        return jsonData

    def _controlJsonData(self):
        if self.controlNotice == -1:
            return None

        data = self.controlNotice

        self.controlNotice = -1
        return data

    def getControlJsonData(self):
        controlData = self._controlJsonData()

        data = {'identity': 3,
                'controlNotice': controlData}

        self.controlDataReset()
        jsonData = json.dumps(data)
        return jsonData

    def _thresholdJsonData(self):
        if self.threshold == []:
            return None

        data = self.threshold

        self.threshold = []
        return data

    def getThresholdJsonData(self):
        thresholdList = self._thresholdJsonData()

        data = {'identity': 4,
                'thresholdList': thresholdList}

        self.thresdDataReset()
        jsonData = json.dumps(data)
        return jsonData

    def getJsonData(self):
        controlData = self._controlJsonData()
        thresholdData = self._thresholdJsonData()
        sensorData = self._sensorJsonData()

        if (controlData is None and thresholdData is None and sensorData is None):
            return None
        if controlData is not None:
            self.controlDataReset()
        if thresholdData is not None:
            self.thresdDataReset()
        if sensorData is not None:
            self.sensorDataReset()

        data = {'identity': 1,
                'control': controlData,
                'threshold': thresholdData,
                'sensor': sensorData}
        jsonData = json.dumps(data)
        return jsonData

class ReceiveBuffer:
    def __init__(self):
        self.controlFlag = False
        self.thresholdFlag = False

        self.cData = bytes([0])
        self.tInfo = bytes([])

    def changeControlValue(self, value):
        self.controlValue = value
        self.controlFlag = True

    def changeThresholdList(self, lst):
        self.thresholdList = lst
        self.thresholdFlag = True

    def stealControlValue(self):
        if self.controlFlag is True:
            print("-------------------------------------------------------------{}".format(self.controlValue))
            self.controlFlag = False
            return self.controlValue
        return None

    def stealThresList(self):
        if self.thresholdFlag is True:
            self.thresholdFlag = False
            return self.thresholdList
        return None

