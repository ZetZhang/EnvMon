import queue
import json

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
        self.lightIntensity = ""

    def setTemperature(self, value):
        if isinstance(value, bytes):
            self.temperature = str(value)

    def setHumidity(self, value):
        if isinstance(value, bytes):
            self.humidity = str(value)

    def setPressure(self, value):
        if isinstance(value, bytes):
            self.pressure = str(value)

    def setSample(self, value):
        if isinstance(value, bytes):
            self.sample = str(value)

    def setLightIntensity(self, value):
        if isinstance(value, bytes):
            self.lightIntensity = str(value)

    def setControlNotice(self, value):
        self.controlFlag = True
        if isinstance(value, bytes):
            self.controlNotice = str(value)

    def setThreshold(self, value):
        self.thresholdFlag = True
        if isinstance(value, bytes):
            self.threshold = str(value)

    def getSensorJsonData(self):
        if self.isSensorDataReady():
            data = [{'temperature': self.temperature},
                    {'humidity': self.humidity},
                    {'pressure': self.humidity},
                    {'lightintensity': self.lightIntensity},
                    {'controlnotice': self.controlNotice},
                    {'threshold': self.threshold}]

            jsonData = json.dumps(data)
            return jsonData
        return None

    def getControlJsonData(self):
        if self.controlNotice == "":
            return None
        data = {'controlNotice': self.controlNotice}
        jsonData = json.dumps(data)
        self.controlNotice = ""
        return jsonData

    def getThresholdJsonData(self):
        if self.threshold == "":
            return None
        data = {'thresholdList': self.threshold}
        jsonData = json.dumps(data)
        self.threshold = ""
        return jsonData

    def getJsonData(self):
        controlData = self.getControlJsonData()
        thresholdData = self.getThresholdJsonData()
        sensorData = self.getSensorJsonData()

        if (controlData is None and thresholdData is None and sensorData is None):
            return None

        data = {'identity': 1,
                'control': self.getControlJsonData(),
                'threshold': self.getThresholdJsonData(),
                'sensor': self.getSensorJsonData()}
        jsonData = json.dumps(data)
        return jsonData
