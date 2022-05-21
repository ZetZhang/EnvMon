
short sampleBuffer[512];
volatile int samplesRead;
LightingUtils *lU_SA = NULL;

//声音采集，使用单通道数据
//计算分贝：选取值最大的一个作为当前的环境噪声，对该最大值乘5，再取以10为底的对数，乘24即为当前的环境噪声（分贝）。
void onPDMdata() {
  int bytesAvailable = PDM.available();  //查询可用字节数；
  PDM.read(sampleBuffer, bytesAvailable);  //存入缓冲区；
  samplesRead = bytesAvailable / 2;
}

void test_start() {
  // 环境温度湿度
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
  
  // 大气压强
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }

  // 日照强度
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
  }
//  digitalWrite(LED_PWR, LOW);
  APDS.setGestureSensitivity(90);

  // 环境噪声
  PDM.onReceive(onPDMdata);//初始化麦克风MP34DT05；
  PDM.begin(1, 16000);

  // 摇一摇
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  lU_SA = LightingUtils::LightingUtilsInstance();
}

void test_load() {
  // 环境温度湿度
  float temperature = HTS.readTemperature(CELSIUS); // CELSIUS | FAHRENHEIT
  float humidity = HTS.readHumidity();//读取湿度； 
  Serial.println("Temperature = " + String(temperature) + " °C");
  Serial.println("Humidity = " + String(humidity) + " %");
  
  // 大气压强
  float pressure = BARO.readPressure(KILOPASCAL); // PSI | MILLIBAR | KILOPASCAL
  Serial.println("Pressure = " + String(pressure) + " kPa");

  // 日照强度
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
  }
  while (! APDS.colorAvailable()) { delay(5); }
  if (APDS.colorAvailable()) {
    int r, g, b, a;
    APDS.readColor(r, g, b, a);
    Serial.print("r = " + String(r) + ", g = " + String(g) + ", b = " + String(b) + ", a = " + String(a) + " | ");
    if(a > 15) {
      Serial.println("白天");
      lU_SA->setPowOn();
    } else {
      Serial.println("夜晚");
      lU_SA->setPowOff();
    }
  }
  
  // Gesture
  if (APDS.gestureAvailable()) {
    int gesture = APDS.readGesture();
    switch (gesture) {
      case GESTURE_UP:
        Serial.println("Detected UP gesture");
        lU_SA->setPinOn();
        break;
      case GESTURE_DOWN:
        Serial.println("Detected DOWN gesture");
        lU_SA->setPinOff();
        break;
      case GESTURE_LEFT:
        Serial.println("Detected LEFT gesture");
        break;
      case GESTURE_RIGHT:
        Serial.println("Detected RIGHT gesture");
        break;
      default:
        // Ignore.
        break;
    }
  }

  // 环境噪声
  while (!samplesRead);
  uint32_t sound_max = 0;
  int i;
  for (i = 0; i < samplesRead; i++) {
    if(sampleBuffer[i] < 0)
      sampleBuffer[i] = -sampleBuffer[i];
    if(sampleBuffer[i] > sound_max) 
      sound_max= sampleBuffer[i];  
  }
  uint32_t sound = 24*log10(sound_max*5); //得出分贝；
  Serial.println("sound: " + String(sound) + " dB");

  // 摇一摇
  float x, y, z;
  if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
      Serial.println(String(x) + '\t' + String(y) + '\t' + String(z));
  }

  Serial.println();
}
