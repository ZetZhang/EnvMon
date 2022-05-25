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
        else:
            self.temperature = str(value)

    def setHumidity(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.humidity = str(res)
        else:
            self.humidity = str(value)

    def setPressure(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('f', value)
            res = round(val, 2)
            self.pressure = str(res)
        else:
            self.pressure = str(value)

    def setSample(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('b', value)
            self.sample = str(val)
        else:
            self.sample = str(value)

    def setLightIntensity(self, value):
        if isinstance(value, bytes):
            [val] = struct.unpack('b', value)
            self.lightIntensity = str(val)
        else:
            self.lightIntensity = str(value)

    def setControlNotice(self, value):
        self.controlFlag = True
        self.controlNotice = str(value)

    def setThreshold(self, value):
        self.thresholdFlag = True
        self.threshold = str(value)

    def _sensorJsonData(self):
        if self.isSensorDataReady():
            data = [{'temperature': self.temperature},
                    {'humidity': self.humidity},
                    {'pressure': self.pressure},
                    {'sample': self.sample},
                    {'lightintensity': self.lightIntensity}
                    ]

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
        if self.controlNotice == "":
            return None

        data = {'controlNotice': self.controlNotice}

        self.controlNotice = ""
        return data

    def getControlJsonData(self):
        controlData = self._controlJsonData()

        data = {'identity': 3,
                'controlNotice': controlData}

        self.controlDataReset()
        jsonData = json.dumps(data)
        return jsonData

    def _thresholdJsonData(self):
        if self.threshold == "":
            return None

        data = {'thresholdList': self.threshold}

        self.threshold = ""
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
        self.tInfo = bytes([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    def changeControlValue(self, value):
        if isinstance(value, str):
            value = value.encode('utf-8')
        self.controlValue = value
        self.controlFlag = True

    def changeThresholdList(self, list):
        self.thresholdList = list
        self.thresholdFlag = True

    def stealControlValue(self):
        if self.controlFlag is True:
            self.controlFlag = False
            return self.controlValue
        return None

    def stealThresList(self):
        if self.stealControlValue is True:
            self.thresholdFlag = False
            return self.thresholdList
        return None

