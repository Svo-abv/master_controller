#include "App.h"  // Подключаем заголовочный файл

// --- Глобальные объекты ---

const char *telegramAddress = "api.telegram.org";

const char *root_ca = R"rawliteral(
-----BEGIN CERTIFICATE-----
MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx
EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT
EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp
ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz
NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH
EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE
AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw
DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD
E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH
/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy
DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh
GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR
tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA
AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE
FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX
WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu
9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr
gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo
2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO
LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI
4uJEvlz36hz1
-----END CERTIFICATE-----
)rawliteral";

const char *loginIndex = R"rawliteral(
<form name='loginForm'>
<table width='20%' bgcolor='A09F9F' align='center'>
<tr>
<td colspan=2>
<center><font size=4><b>ESP32 Login Page</b></font></center>
<br>
</td>
<br>
<br>
</tr>
<td>Username:</td>
<td><input type='text' size=25 name='userid'><br></td>
</tr>
<br>
<br>
<tr>
<td>Password:</td>
<td><input type='Password' size=25 name='pwd'><br></td>
<br>
<br>
</tr>
<tr>
<td><input type='submit' onclick='check(this.form)' value='Login'></td>
</tr>
</table>
</form>
<script>
function check(form)
{
if(form.userid.value=='" OTA_USER "' && form.pwd.value=='" OTA_PASSWORD "')
{
window.open('/serverIndex')
}
else
{
 alert('Error Password or Username')/*displays error message*/
}
}
</script>
)rawliteral";

const char *serverIndex = R"rawliteral(
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
<input type='file' name='update'>
<input type='submit' value='Update'>
</form>
<div id='prg'>progress: 0%</div>
<script>
$('form').submit(function(e){
e.preventDefault();
var form = $('#upload_form')[0];
var data = new FormData(form);
 $.ajax({
url: '/update',
type: 'POST',
 data,
contentType: false,
processData:false,
xhr: function() {
var xhr = new window.XMLHttpRequest();
xhr.upload.addEventListener('progress', function(evt) {
if (evt.lengthComputable) {
var per = evt.loaded / evt.total;
$('#prg').html('progress: ' + Math.round(per*100) + '%');
}
}, false);
return xhr;
},
success:function(d, s) {
console.log('success!')
},
error: function (a, b, c) {
}
});
});
</script>
)rawliteral";

// --- Глобальный экземпляр приложения ---
App app;

// --- Реализация статических функций для ISR ---
void IRAM_ATTR static_start_communicate() { app.startCommunicate(); }

void IRAM_ATTR static_set_flag() { app.setFlag(); }

// --- Реализация методов класса App ---

App::App()
    : HttpServer(SERVER_PORT),
      http(),
      ntpUDP(),
      timeClient(ntpUDP, "ntp.msk-ix.ru", 18000),
      timer(nullptr),
      timerMux(portMUX_INITIALIZER_UNLOCKED),
      run_now(false),
      is_pause(false),
      lastExec(0),
      openThermTimer(nullptr),
      openThermTimerMux(portMUX_INITIALIZER_UNLOCKED),
      ot(inPin, outPin),
      responseStatus(OpenThermResponseStatus::NONE),
      communicateNow(false),
      dht1(DHT_PIN1, DHT22),
      dht2(DHT_PIN2, DHT22),
      ads_temp(),
      messageId(0),
      telegramStatus(0),
      lastTemp(0),
      Rs(1096),
      Vcc(3.3),
      globalRoomTemp(23.0),
      targetRoomTemp(23.0),
      boilerCurrTemp(60.0),
      boilerMinTemp(35.0),
      boilerMaxTemp(52.0),
      enableCentralHeating(false),
      enableHotWater(false),
      enableCooling(false),
      enableOTCCompensation(true),
      gasCrash(0x0),
      gasCrashFlags(0x0),
      crashRaw(0),
      targetHCTemp(60),
      currentTargetHCTemp(60),
      HCTemp(60),
      outside(0),
      isFlame(false),
      modulation(0),
      maxCapacity(0),
      minModulation(0) {
   // Инициализация переменных, которые зависят от констант
   pin1 = 0;
   pin2 = 0;
   pin3 = 0;
   pin4 = 0;
   pin5 = 0;
   pin6 = 0;
}

void App::handle() {
   ArduinoOTA.handle();
   WebSerial.loop();

   if (communicateNow) {
      communicateBoiler();
      portENTER_CRITICAL(&openThermTimerMux);
      communicateNow = false;
      portEXIT_CRITICAL(&openThermTimerMux);
   }

   if (run_now && !is_pause) {
      one_blink();
      unsigned int msecLast = millis();
      collectAndSend();
      unsigned int nowmsec = millis();
      lastExec = (nowmsec - msecLast) / 1000;
      DEBUG_MSGLN(lastExec);
      DEBUG_MSGLN(String(ESP.getFreeHeap()));
      three_blink();

      portENTER_CRITICAL(&timerMux);
      run_now = false;
      portEXIT_CRITICAL(&timerMux);
      timerWrite(timer, 0);
      timerAlarmEnable(timer);
   }
}

void App::init() {
   DEBUG_MSGLN("Network..");
   initNetwork();

   DEBUG_MSGLN("HTTP...");
   initHTTPServer();

   DEBUG_MSGLN("DTH...");

   dht1.begin();
   dht2.begin();

   DEBUG_MSGLN("EEPROM...");
   app.initEEPROM();

   DEBUG_MSGLN("OTA...");
   app.initOTA();

   DEBUG_MSGLN("ADC...");
   app.initADC();

   DEBUG_MSGLN("NTP...");
   app.initTimeClient();

   DEBUG_MSGLN("OpenTherm...");
   app.initOpenTherm();

   DEBUG_MSGLN("Timers...");
   app.initTimers();
}

// --- OpenTherm ---
void IRAM_ATTR App::startCommunicate() {
   portENTER_CRITICAL(&openThermTimerMux);
   communicateNow = true;
   portEXIT_CRITICAL(&openThermTimerMux);
}

void App::communicateBoiler() {
   DEBUG_MSGLN("communicateBoiler");
   DEBUG_MSGLN("responseStatus " + String((int)responseStatus));
   if (responseStatus == OpenThermResponseStatus::SUCCESS) {

      if ((globalRoomTemp - 0.2) < targetRoomTemp ||
          boilerCurrTemp < boilerMinTemp) {
         if (targetHCTemp != currentTargetHCTemp) {
            targetHCTemp = currentTargetHCTemp;
            enableCentralHeating = true;
            DEBUG_MSGLN("Start setBoilerTemperature " + String(globalRoomTemp) +
                        " " + String(targetRoomTemp) + " " +
                        String(boilerCurrTemp) + " " + String(targetHCTemp) +
                        " " + String(currentTargetHCTemp));  // 1
            ot.setBoilerTemperature(targetHCTemp);
         }
      } else if (globalRoomTemp > targetRoomTemp &&
                 boilerCurrTemp >= boilerMaxTemp) {
         if (targetHCTemp != 1) {
            targetHCTemp = 1;
            enableCentralHeating = false;
            DEBUG_MSGLN("Stop setBoilerTemperature " + String(globalRoomTemp) +
                        " " + String(targetRoomTemp) + " " +
                        String(boilerCurrTemp) + " " + String(targetHCTemp) +
                        " " + String(currentTargetHCTemp));  // 2
            ot.setBoilerTemperature(targetHCTemp);
         }
      }

      DEBUG_MSGLN("setBoilerStatus, enableCentralHeating " +
                  String(enableCentralHeating) + " enableHotWater " +
                  String(enableHotWater) + " enableCooling " +
                  String(enableCooling) + " enableOTCCompensation " +
                  String(enableOTCCompensation));
      unsigned long resp1 =
          ot.setBoilerStatus(enableCentralHeating, enableHotWater,
                             enableCooling, enableOTCCompensation);

      DEBUG_MSGLN("Central Heating: " +
                  String(ot.isCentralHeatingActive(resp1) ? "on" : "off"));
      DEBUG_MSGLN("Hot Water: " +
                  String(ot.isHotWaterActive(resp1) ? "on" : "off"));
      DEBUG_MSGLN("Flame: " + String(ot.isFlameOn(resp1) ? "on" : "off"));

      responseStatus = ot.getLastResponseStatus();

      isFlame = ot.isFlameOn(resp1);

      HCTemp = ot.getBoilerTemperature();

      modulation = ot.getModulation();

      uint16_t resp2 = ot.sendRequest(
          ot.buildRequest(OpenThermMessageType::READ_DATA,
                          OpenThermMessageID::MaxCapacityMinModLevel, 0));
      maxCapacity = (resp2 >> 8) & 0xff;
      minModulation = (uint8_t)resp2 & 0xff;

      resp1 = ot.sendRequest(ot.buildRequest(OpenThermMessageType::READ_DATA,
                                             OpenThermMessageID::Toutside, 0));
      outside = ot.getFloat(resp1);

      // data = ot.temperatureToData(22);
      // boilerRoomTemp = globalRoomTemp;
      // setRoomTemp =
      // ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA,
      // OpenThermMessageID::Tr, data));
      // msgTypeRoomTemp = (setRoomTemp << 1) >> 29;

      resp2 = ot.sendRequest(ot.buildRequest(OpenThermMessageType::READ_DATA,
                                             OpenThermMessageID::ASFflags, 0));
      gasCrash = (resp2 >> 8) & 0xff;

      gasCrashFlags = (uint8_t)resp2;
      crashRaw = resp2;
      // gasCrash = ot.getFault();
   } else {
      initOpenTherm();
   }
}

void App::initOpenTherm() {
   ot.begin();

   unsigned long response =
       ot.setBoilerStatus(false, false, false, enableOTCCompensation);
   responseStatus = ot.getLastResponseStatus();
   if (responseStatus == OpenThermResponseStatus::SUCCESS) {
      DEBUG_MSGLN("Central Heating: " +
                  String(ot.isCentralHeatingActive(response) ? "on" : "off"));
      DEBUG_MSGLN("Hot Water: " +
                  String(ot.isHotWaterActive(response) ? "on" : "off"));
      DEBUG_MSGLN("Flame: " + String(ot.isFlameOn(response) ? "on" : "off"));

      DEBUG_MSGLN("initOpenTherm setBoilerTemperature " +
                  String(globalRoomTemp) + " " + String(targetRoomTemp) + " " +
                  String(boilerCurrTemp) + " " + String(targetHCTemp) + " " +
                  String(currentTargetHCTemp));  // 3
      ot.setBoilerTemperature(targetHCTemp);
      // setHCTemp =
      // ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA,
      // OpenThermMessageID::TSet, data));
      // msgTypeHCTemp = (setHCTemp << 1) >> 29;
      // ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA,
      // OpenThermMessageID::TrSet,targetRoomTemp));
   }
   if (responseStatus == OpenThermResponseStatus::NONE) {
      DEBUG_MSGLN("Error: OpenTherm is not initialized");
   } else if (responseStatus == OpenThermResponseStatus::INVALID) {
      DEBUG_MSGLN("Error: Invalid response " + String(response, HEX));
   } else if (responseStatus == OpenThermResponseStatus::TIMEOUT) {
      DEBUG_MSGLN("Error: Response timeout");
   }
}

// --- Web Server ---
void App::initHTTPServer() {
   HttpServer.onNotFound(
       std::bind(&App::handleNotFound, this, std::placeholders::_1));
   HttpServer.on(
       "/", std::bind(&App::handleGetMainStatus, this, std::placeholders::_1));
   HttpServer.on("/reboot",
                 std::bind(&App::handleReboot, this, std::placeholders::_1));
   HttpServer.on("/pause",
                 std::bind(&App::handlePause, this, std::placeholders::_1));

   HttpServer.on("/set_mess_id0", [this](AsyncWebServerRequest *request) {
      this->messageId = 0;
      String result = "Succeed! ";
      request->send(200, "text/html", result);
      delay(1000);
   });

   HttpServer.on(
       "/set-room-temp", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          request->send(200, "text/html", "vars is set");
       },
       nullptr,
       std::bind(&App::handleSetTargetRoomTemp, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3,
                 std::placeholders::_4, std::placeholders::_5));

   HttpServer.on(OTA_PATH, HTTP_GET, [this](AsyncWebServerRequest *request) {
      request->send(200, "text/html", loginIndex);
   });
   HttpServer.on("/serverIndex", HTTP_GET,
                 [this](AsyncWebServerRequest *request) {
                    request->send(200, "text/html", serverIndex);
                 });
   /*handling uploading firmware file */
   HttpServer.on(
       "/update", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
          ESP.restart();
       },
       std::bind(&App::handleUpload, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3,
                 std::placeholders::_4, std::placeholders::_5,
                 std::placeholders::_6));

   HttpServer.begin();
   WebSerial.begin(&HttpServer);
}

void App::handleNotFound(AsyncWebServerRequest *request) {
   request->send(404, "text/plain", "404: Not found");
}

void App::handlePause(AsyncWebServerRequest *request) {
   request->send(200, "text/html", "Succeed! " + !is_pause);
   is_pause = !is_pause;
   delay(1000);
}

void App::handleReboot(AsyncWebServerRequest *request) {
   request->send(200, "text/html", "Succeed! Now rebooting...");
   delay(1000);
   ESP.restart();
}

void App::handleGetMainStatus(AsyncWebServerRequest *request) {
   JSONVar json;
   byte ar[6];
   WiFi.macAddress(ar);
   char macAddr[18];
   sprintf(macAddr, "%2X:%2X:%2X:%2X:%2X:%2X", ar[0], ar[1], ar[2], ar[3],
           ar[4], ar[5]);
   json["text"] = "in progress...";
   json["IP"] = WiFi.localIP().toString();
   json["MAC"] = macAddr;
   json["RSSI"] = WiFi.RSSI();
   json["dev_mode"] = dev;
   json["SDK"] = ESP.getSdkVersion();
   json["FlashChipSpeed"] = String(ESP.getFlashChipSpeed());
   json["FlashChipSize"] = String(ESP.getFlashChipSize());
   json["FreeHeap"] = String(ESP.getFreeHeap());
   json["ChipTemp"] = (temprature_sens_read() - 32) / 1.8;
   json["Hall"] = hallRead();
   json["Last_cycle_time"] = lastExec;
   json["ADC_pin1"] = pin1;
   json["ADC_pin2"] = pin2;
   json["ADC_pin3"] = pin3;
   json["ADC_pin4"] = pin4;
   json["ADC_pin5"] = pin5;
   json["ADC_pin6"] = pin6;
   json["Message_id"] = messageId;
   json["telegram_status"] = telegramStatus;
   json["telegram_last_message"] = telegramLastMessage;
   json["telegram_response"] = telegramResponse;
   json["globalRoomTemp"] = globalRoomTemp;
   json["targetRoomTemp"] = targetRoomTemp;
   json["boilerMinTemp"] = boilerMinTemp;
   json["boilerMaxTemp"] = boilerMaxTemp;
   json["boilerCurrTemp"] = boilerCurrTemp;
   json["targetHCTemp"] = targetHCTemp;
   json["currentTargetHCTemp"] = currentTargetHCTemp;
   json["isFlame"] = isFlame;
   json["boiler"] = String((byte)responseStatus);
   timeClient.update();  // Используем this->timeClient
   json["Time"] = timeClient.getFormattedTime();
   request->send(200, "application/json", JSON.stringify(json));
}

void App::handleSetTargetRoomTemp(AsyncWebServerRequest *request, uint8_t *data,
                                  size_t len, size_t index, size_t total) {

   JSONVar json = JSON.parse((const char *)data);

   targetRoomTemp = json["targetRoomTemp"];
   currentTargetHCTemp = json["currentTargetHCTemp"];
   boilerMinTemp = json["boilerMinTemp"];
   boilerMaxTemp = json["boilerMaxTemp"];

   EEPROM.begin(EEPROMSize);
   EEPROM.put(addrTargetRoomTemp, targetRoomTemp);
   EEPROM.put(addrCurrentTargetHCTemp, currentTargetHCTemp);
   EEPROM.put(addrBoilerMinTemp, boilerMinTemp);
   EEPROM.put(addrBoilerMaxTemp, boilerMaxTemp);
   EEPROM.commit();
   EEPROM.end();
}

void App::handleUpload(AsyncWebServerRequest *request, String filename,
                       size_t index, uint8_t *data, size_t len, bool final) {

   if (!index) {
      Serial.printf("Update: %s\n", filename.c_str());
      if (!Update.begin(
              UPDATE_SIZE_UNKNOWN)) {  // start with max available size
         Update.printError(Serial);
      }
   }

   if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
         Update.printError(Serial);
      }
   }

   if (final) {
      if (Update.end(true)) {  // true to set the size to the current progress
         Serial.printf("Update Success: %u\nRebooting...\n", index + len);
      } else {
         Update.printError(Serial);
      }
   }
}

// --- ADC ---
int App::AnalogRead(int pin, int iter, int timeout) {
   unsigned int val = 0;
   for (int i = 0; i < iter; i++) {
      val += analogRead(pin);
      delay(timeout);
   }

   val = val / iter;
   return val;
}

int App::AnalogReadRaw(adc1_channel_t pin, int iter, int timeout) {
   uint64_t val = 0;
   for (int i = 0; i < iter; i++) {
      val += adc1_get_raw(pin);
      delay(timeout);
   }

   return val / iter;
}

int App::AnalogReadAvg(adc1_channel_t pin, int iter, int timeout) {
   uint64_t val = 0;
   for (int i = 0; i < iter; i++) {
      val += adc1_get_raw(pin);
      delay(timeout);
   }
   return val / iter;
}

int App::AnalogReadMax(adc1_channel_t pin, int iter, int timeout) {
   uint64_t val = 0;
   for (int i = 0; i < iter; i++) {
      uint64_t curr = adc1_get_raw(pin);
      if (curr > val) val = curr;
      delay(timeout);
   }

   return val;
}

int App::AnalogReadTrueRMS(adc1_channel_t pin, int iter, int timeout) {
   uint64_t val = 0;
   for (int i = 0; i < iter; i++) {
      uint64_t curr = adc1_get_raw(pin);
      // if(curr>val)
      val += curr * curr;
      delay(timeout);
   }

   return sqrt(val / iter);
}

int App::AnalogReadExtAdsTemp(int ended) {
   uint64_t val = 0;
   for (int i = 0; i < 50; i++) {

      val += ads_temp.readADC_SingleEnded(ended);
      delay(1);
   }
   return val / 50;
}

// --- NTC ---
double App::Thermister(int val) {
   DEBUG_MSGLN("Thermister");

   DEBUG_MSGLN(val);
   double V_NTC = (double)val * 0.000125;  //* 0.000805860806;  // /1023
   DEBUG_MSGLN(V_NTC);
   double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
   DEBUG_MSGLN(R_NTC);
   R_NTC = log(R_NTC);
   double Temp = 1 / (kA + (kB + (kC * R_NTC * R_NTC)) * R_NTC);
   Temp = Temp - 273.15;
   return Temp;
}

double App::boilerThermister(int val) {
   DEBUG_MSGLN("boilerThermister");
   DEBUG_MSGLN(val);
   double V_NTC = (double)val * 0.000125;  //* 0.000805860806;  // /1023
   DEBUG_MSGLN(V_NTC);
   double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
   DEBUG_MSGLN(R_NTC);
   R_NTC = log(R_NTC);
   double Temp = 1 / (bkA + (bkB + (bkC * R_NTC * R_NTC)) * R_NTC);
   Temp = Temp - 273.15;
   return Temp;
}

// --- Ticker ---
void IRAM_ATTR App::setFlag() {
   portENTER_CRITICAL(&timerMux);
   run_now = true;
   portEXIT_CRITICAL(&timerMux);
}

// --- Telegram ---
void App::collectAndSend() {

   String tempHome = getVoltageData();
   String tempOutput = getSensorData(dht1, "Котельная:");

   String tempStreet = getSensorData(dht2, "Улица:");
   two_blink();  // 1
   String tempInput = getHeatCarrierData();
   two_blink();  // 2
   String tempBoiler = getBoilerData();

   String tempFloors;
   tempFloors += getFloorData("http://192.168.0.11/data", "Подвал:");
   tempFloors +=
       getFloorData("http://192.168.0.16/data", "Первый этаж, кухня:", true);
   tempFloors += getFloorData("http://192.168.0.10/data",
                              "Второй этаж, спальня большая:");

   two_blink();  // 3
   WiFiClientSecure httpsClient;
   httpsClient.setCACert(root_ca);
   httpsClient.setTimeout(3);

   DEBUG_MSGLN("httpsClient.connect(telegramAddress, 443):");
   if (httpsClient.connect(telegramAddress, 443)) {

      DEBUG_MSGLN("connected");

      String postData, Link;

      two_blink();  // 4

      Link = "https://" + String(telegramAddress) + "/bot" + botToken;
      postData = "chat_id=" + String(chat_id);

      if (messageId == 0) {
         Link += "/SendMessage";
         postData += "&disable_notification=1";
      } else {
         Link += "/editMessageText";
         postData += "&message_id=" + String(messageId);
      }
      postData += "&parse_mode=HTML&text=" + tempHome + "-------\r\n" +
                  tempStreet + "-------\r\n" + tempFloors + "-------\r\n" +
                  tempOutput + "-------\r\n" + tempInput + "-------\r\n" +
                  tempBoiler + "-------\r\n\r\n" +
                  String("Время запуска запуска: <b>") + time_d_start +
                  "</b>\r\n" + String("Обновлено в: ") +
                  timeClient.getFormattedTime() + "\r\n" +
                  "<i>heap: " + String(ESP.getFreeHeap()) + "</i>\r\n";

      String req =
          String("GET ") + Link + " HTTP/1.1\r\n" + "Host: " + telegramAddress +
          "\r\n" + "Content-Type: application/x-www-form-urlencoded\r\n" +
          "Connection: keep-alive\r\n" +
          "Content-Length: " + postData.length() + "\r\n" + "\r\n" + postData;
      telegramLastMessage = req;
      httpsClient.println(req);

      // two_blink();  // 5
      String line;
      // while (httpsClient.connected()) {
      //    line = httpsClient.readStringUntil('\n');
      //    if (line == "\r") {
      //       // Serial.println("headers received");
      //       break;
      //    }
      // }

      DEBUG_MSGLN("reply was:");
      DEBUG_MSGLN("==========");

      two_blink();  // 6
      while (httpsClient.available()) {
         line = httpsClient.readStringUntil('\n');  // Read Line by Line
         DEBUG_MSGLN(line);                         // Print response
      }

      DEBUG_MSGLN("==========");
      DEBUG_MSGLN("\r\n");

      telegramResponse = line;
      JSONVar reply_json = JSON.parse(line);

      if (JSON.typeof(reply_json) != "undefined") {
         if ((bool)reply_json["ok"] == true && messageId == 0) {
            messageId = (int)reply_json["result"]["message_id"];
            EEPROM.begin(EEPROMSize);
            EEPROM.put(addrMessageId, messageId);
            EEPROM.commit();
            EEPROM.end();
            DEBUG_MSGLN(messageId);

         } else if ((bool)reply_json["ok"] == false) {
            messageId = 0;
            char *buf = new char[64];
            telegramStatus = httpsClient.lastError(buf, sizeof(buf));
            delete buf;
            DEBUG_MSGLN("Connection failed!");
         }
      }
   }

   two_blink();  // 7
   httpsClient.flush();
   httpsClient.stop();
}

String App::getHeatCarrierData() {
   double outCurrTemp =
       (double)round(Thermister(AnalogReadExtAdsTemp(0)) * 10) / 10;
   double onCurrTemp =
       (double)round(Thermister(AnalogReadExtAdsTemp(2)) * 10) / 10;

   boilerCurrTemp =
       (double)round(boilerThermister(AnalogReadExtAdsTemp(3)) * 10) / 10;

   String smile;
   if (outCurrTemp > lastTemp)
      smile = "\xF0\x9F\x94\xA5";
   else if (outCurrTemp < lastTemp)
      smile = "\xE2\x9D\x84";
   else
      smile = "";
   lastTemp = outCurrTemp;

   String curr_ten = "вкл.";

   return String("Система:\r\n") + String("подача: <b>") + outCurrTemp +
          "°C</b> " + smile + "\r\n" + String("обратка: <b>") + onCurrTemp +
          "°C</b>\r\n" + String("бойлер: <b>") + boilerCurrTemp + "°C</b>\r\n";
}

String App::getSensorData(DHT &dh, String title) {
   float h = dh.readHumidity();
   float t = dh.readTemperature();
   float i = dh.computeHeatIndex(h, t, false);

   return title + "\r\n" + String("Влажность: <b>") + h + "%</b>\r\n" +
          String("Температура: <b>") + t + "°C</b>\r\n" +
          String("Тепловой индекс: <b>") + i + "°C\\t</b>\r\n";
}

String App::getVoltageData() {
   double L1 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_7, 1000) * 0.000805860806;
   double L2 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_4, 1000) * 0.000805860806;
   double L3 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_6, 1000) * 0.000805860806;

   return "Дом:\r\n" + String("Фаза 1: <b>") +
          (int)round(L1 * kL1) +  // Используем глобальную kL1
          "V</b>\r\n" + String("Фаза 2: <b>") +
          (int)round(L2 * kL2) +  // Используем глобальную kL2
          "V</b>\r\n" + String("Фаза 3: <b>") +
          (int)round(L3 * kL3) +  // Используем глобальную kL3
          "V</b>\r\n";
}

String App::getBoilerData() {
   String boiler;
   portENTER_CRITICAL(&openThermTimerMux);
   boiler = String("Котёл:") + (isFlame ? " \xF0\x9F\x94\xA5" : "") + "\r\n" +
            String("комната: <b>") + globalRoomTemp + "°C</b>\r\n" +
            String("комната, задано: <b>") + targetRoomTemp + "°C</b>\r\n" +
            String("улица: <b>") + outside + "°C</b>\r\n" +
            String("подача: <b>") + HCTemp + "°C</b> " + "\r\n" +
            String("подача, задано: <b>") + targetHCTemp + "°C</b> " + "\r\n" +
            String("модуляция пламени: <b>") + modulation + "%</b>\r\n" +
            String("минимальный уровень модуляции: <b>") + minModulation +
            "%</b>\r\n" + String("максимальная мощность котла: <b>") +
            maxCapacity + "Kw</b>\r\n" + String("Отопление: <b>") +
            enableCentralHeating + "</b>\r\n" + String("Охлаждение: <b>") +
            enableCooling + "</b>\r\n" + String("Горячая вода: <b>") +
            enableHotWater + "</b>\r\n";
   if (gasCrash) {
      boiler += "\xE2\x9B\x94" +
                String("<b>ВНИМАНИЕ! авария на газовом котле. Код: ") +
                String(gasCrash) + "</b>\r\n" + String("флаги: ") +
                String(gasCrashFlags) + "\r\n" + String("сообщение: ") +
                String(crashRaw) + "\r\n";
   }
   portEXIT_CRITICAL(&openThermTimerMux);
   return boiler;
}

String App::getFloorData(String url, String title) {
   return getFloorData(url, title, false);
}

String App::getFloorData(String url, String title, boolean setGlobal) {
   String floor;

   if (http.begin(url)) {  // Теперь используем this->http
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
         String payload = http.getString();
         JSONVar floorData = JSON.parse(payload);
         floor = +"-------\r\n" + title + "\r\n" + String("Температура: <b>") +
                 String((double)floorData["temperature"]) + "°C</b>\r\n" +
                 String("Влажность: <b>") +
                 String((double)floorData["humidity"]) + "%</b>\r\n" +
                 String("Тепловой индекс: <b>") +
                 String((double)floorData["heatIndex"]) + "°C/t</b>\r\n";
         if (setGlobal) {
            globalRoomTemp = (double)floorData["temperature"];
         }
      } else {
         floor = String("Error, code: ") + String((int)httpCode) + "\r\n";
      }
      http.end();
   }
   return floor;
}

// --- Indication ---
void App::one_blink() {
   delay(500);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
}

void App::two_blink() {
   delay(500);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
   delay(100);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
}

void App::three_blink() {
   delay(500);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
   delay(100);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
   delay(100);
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
}

// --- Init ---
void App::initNetwork() {
   DEBUG_MSGLN("Wi-Fi...");

   WiFi.mode(WIFI_STA);
   wl_status_t status = WiFi.begin(ssid, pass);

   if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      DEBUG_MSGLN("Error status, " + String(status));
   }

   DEBUG_MSGLN(WiFi.localIP());
   DEBUG_MSGLN(WiFi.gatewayIP());
   DEBUG_MSGLN(WiFi.subnetMask());
   DEBUG_MSGLN(WiFi.dnsIP());
}

void App::initOTA() {
   ArduinoOTA
       .onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH)
             type = "sketch";
          else  // U_SPIFFS
             type = "filesystem";

          // NOTE: if updating SPIFFS this would be the place to unmount
          // SPIFFS using SPIFFS.end()
          DEBUG_MSGLN("Start updating " + type);
       })
       .onEnd([]() { DEBUG_MSGLN("\nEnd"); })
       .onProgress([](unsigned int progress, unsigned int total) {
          DEBUG_MSGLN(String("Progress: %u%%\r", (progress / (total / 100))));
       })
       .onError([](ota_error_t error) {
          DEBUG_MSGLN(String("Error[%u]: ", error));
          if (error == OTA_AUTH_ERROR) {
             DEBUG_MSGLN("Auth Failed");
          } else if (error == OTA_BEGIN_ERROR) {
             DEBUG_MSGLN("Begin Failed");
          } else if (error == OTA_CONNECT_ERROR) {
             DEBUG_MSGLN("Connect Failed");
          } else if (error == OTA_RECEIVE_ERROR) {
             DEBUG_MSGLN("Receive Failed");
          } else if (error == OTA_END_ERROR) {
             DEBUG_MSGLN("End Failed");
          }
       });

   ArduinoOTA.begin();
}

void App::initEEPROM() {
   int tempMessageId = 0;
   double tempTargetRoomTemp = 0;
   double tempCurrentTargetHCTemp = 0;
   double tempBoilerMinTemp = 0;
   double tempBoilerMaxTemp = 0;

   EEPROM.begin(EEPROMSize);
   EEPROM.get(addrMessageId, tempMessageId);
   EEPROM.get(addrTargetRoomTemp, tempTargetRoomTemp);
   EEPROM.get(addrCurrentTargetHCTemp, currentTargetHCTemp);
   EEPROM.get(addrBoilerMinTemp, tempBoilerMinTemp);
   EEPROM.get(addrBoilerMaxTemp, tempBoilerMaxTemp);
   DEBUG_MSGLN(tempMessageId);
   if (tempMessageId > 0) {
      messageId = tempMessageId;
   }
   if (tempTargetRoomTemp > 0) {
      targetRoomTemp = tempTargetRoomTemp;
   }
   if (tempCurrentTargetHCTemp > 0) {
      currentTargetHCTemp = tempCurrentTargetHCTemp;
   }
   if (tempBoilerMinTemp > 0) {
      boilerMinTemp = tempBoilerMinTemp;
   }
   if (tempBoilerMaxTemp > 0) {
      boilerMaxTemp = tempBoilerMaxTemp;
   }
}

void App::initADC() {
   ads_temp.setGain(GAIN_ONE);
   ads_temp.begin(0x48);

   adc1_config_width(ADC_WIDTH_BIT_12);
   adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
   adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);  // 39
   adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  // 34
   adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);  // 35
   adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);  // 32
   adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);  // 33
}

void App::initTimeClient() {
   timeClient.begin();
   timeClient.update();

   while (!timeClient.isTimeSet()) {
      DEBUG_MSG(".");
      delay(1000);
   }

   unsigned long epochTime = timeClient.getEpochTime();
   struct tm *ptm = gmtime((time_t *)&epochTime);

   int currentYear = ptm->tm_year + 1900;
   int currentMonth = ptm->tm_mon + 1;
   int monthDay = ptm->tm_mday;

   time_d_start = String(monthDay) + "." + String(currentMonth) + "." +
                  String(currentYear) + " " + timeClient.getFormattedTime();
}

void App::initTimers() {
   timer = timerBegin(0, 8000, true);
   timerAttachInterrupt(timer, &static_set_flag, true);
   timerAlarmWrite(timer, 600000, false);  // 10 минут
   timerAlarmEnable(timer);

   openThermTimer = timerBegin(1, 8000, true);  // 80MHz / 8000 = 10000 per sec.
   timerAttachInterrupt(openThermTimer, &static_start_communicate, true);
   timerAlarmWrite(openThermTimer, 40000, true);  // 4 sec
   timerAlarmEnable(openThermTimer);
}
