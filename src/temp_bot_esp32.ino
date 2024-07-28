#ifdef __cplusplus
extern "C"
{
#endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

#define RXD2 16 // RXX2 pin
#define TXD2 17 // TX2 pin

#include <SPI.h>

//---------------------------------------Wi-Fi------------------------------------------------------//

#include <WiFi.h>
#define LED 2

int status = WL_IDLE_STATUS;
WiFiUDP ntpUDP;

//---------------------------------------Time------------------------------------------------------//

#include <NTPClient.h>

const long utcOffsetInSeconds = 18000;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
String time_d_start;

//---------------------------------------Firmware mode-----------------------------------------------//
#define DEBUG(x) \
  if (dev == 1)  \
  (x)
#define DEBUG_MSG(x) \
  if (dev == 1)      \
  Serial.print(x)
#define DEBUG_MSGLN(x) \
  if (dev == 1)        \
  Serial.println(x)
#include "init.h"
//---------------------------------------WEB Server-------------------------------------------------//

#include <WebServer.h>
#define SERVER_PORT 80 // Порт для входа, он стандартный 80 это порт http

WebServer HttpServer(SERVER_PORT);

//---------------------------------------OTA Update-------------------------------------------------//

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Update.h>

#if (dev == 1)
const char *host = "esp32_dev";
#else
const char *host = "esp32";
#endif

/*
   Login page
*/

const char *loginIndex =
    "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
    "<tr>"
    "<td colspan=2>"
    "<center><font size=4><b>ESP32 Login Page</b></font></center>"
    "<br>"
    "</td>"
    "<br>"
    "<br>"
    "</tr>"
    "<td>Username:</td>"
    "<td><input type='text' size=25 name='userid'><br></td>"
    "</tr>"
    "<br>"
    "<br>"
    "<tr>"
    "<td>Password:</td>"
    "<td><input type='Password' size=25 name='pwd'><br></td>"
    "<br>"
    "<br>"
    "</tr>"
    "<tr>"
    "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
    "</tr>"
    "</table>"
    "</form>"
    "<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='" OTA_USER "' && form.pwd.value=='" OTA_PASSWORD "')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
    "</script>";

/*
   Server Index Page
*/

const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";

//-----------------------------------JSON----------------------------------------------------------//

#include <Arduino_JSON.h>

//-----------------------------------HTTPS Client--------------------------------------------------//

#include <HTTPClient.h>
HTTPClient http;

//-----------------------------------HTTPS Client--------------------------------------------------//

#include <WiFiClientSecure.h>

const char *root_ca = "-----BEGIN CERTIFICATE-----\n"
                      "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"
                      "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"
                      "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"
                      "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n"
                      "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"
                      "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n"
                      "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n"
                      "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n"
                      "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n"
                      "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n"
                      "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n"
                      "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n"
                      "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n"
                      "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n"
                      "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n"
                      "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n"
                      "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n"
                      "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n"
                      "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n"
                      "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n"
                      "4uJEvlz36hz1\n"
                      "-----END CERTIFICATE-----\n";

//---------------------------------------Telegram and Wi-Fi params---------------------------------//

const char *telegramAddress = "api.telegram.org";

int messageId = 0;

int telegramStatus = 0;

String telegramResponse;
String telegramLastMessage;
//------------------------------------Ticker-----------------------------------------------------//

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool run_now = false;
volatile bool is_pause = false;

int lastExec = 0;

//------------------------------------ADS---------------------------------------------------------//
#include <driver/adc.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads_temp;

double pin1 = 0;
double pin2 = 0;
double pin3 = 0;
double pin4 = 0;
double pin5 = 0;
double pin6 = 0;

//------------------------------------AC Voltmetr-------------------------------------------------//

#define kL1 158.693926
#define kL2 158.397487
#define kL3 159.856864

//------------------------------------DHT Sensor--------------------------------------------------//

#include "DHT.h"

#define DHT_PIN1 16
#define DHT_PIN2 18
DHT dht1(DHT_PIN1, DHT22);
DHT dht2(DHT_PIN2, DHT22);

//------------------------------------NTC params--------------------------------------------------//

unsigned int Rs = 1096;
double Vcc = 3.3;
double lastTemp = 0;
double kA = 0.00158933576950404;
double kB = 0.000244017890892347;
double kC = 0.000000213896311397919;

double bkA = 0.0016086108187479;
double bkB = 0.000274663510997117;
double bkC = 0.000000198074563227963;

//------------------------------------GAS params--------------------------------------------------//

#include <OpenTherm.h>

hw_timer_t *openThermTimer = NULL;
portMUX_TYPE openThermTimerMux = portMUX_INITIALIZER_UNLOCKED;

OpenThermResponseStatus responseStatus;

bool communicateNow = false;

double globalRoomTemp = 23.0;
double targetRoomTemp = 23.0;

double boilerCurrTemp = 60.0;

bool enableCentralHeating = false;
bool enableHotWater = true;
bool enableCooling = false;
bool enableOTCCompensation = true;

uint8_t gasCrash = 0x0;
uint8_t gasCrashFlags = 0x0;

uint16_t crashRaw = 0;

double targetHCTemp = 60;
double currentTargetHCTemp = 60;
double HCTemp = 60;
float outside = 0;

bool isFlame = false;
float modulation = 0;
uint8_t maxCapacity = 0;
uint8_t minModulation = 0;

const int inPin = 13;  // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int outPin = 15; // for Arduino, 5 for ESP8266 (D1), 22 for ESP32
OpenTherm ot(inPin, outPin);

//---------------------------------------EPPROM------------------------------------------------------//

#include <EEPROM.h>

uint EEPROMSize = sizeof(messageId) + sizeof(targetRoomTemp) + sizeof(currentTargetHCTemp);
uint addrMessageId = 0;
uint addrTargetRoomTemp = addrMessageId + sizeof(messageId);
uint addrCurrentTargetHCTemp = addrTargetRoomTemp + sizeof(targetRoomTemp);

void IRAM_ATTR handleInterrupt()
{
  ot.handleInterrupt();
}

void IRAM_ATTR startCommunicate(){
  portENTER_CRITICAL(&openThermTimerMux);
  communicateNow = true;
  portEXIT_CRITICAL(&openThermTimerMux);
}

void communicateBoiler()
{
  enableCentralHeating = true;
  if (responseStatus == OpenThermResponseStatus::SUCCESS)
  {

    if ((targetRoomTemp - 0.2) > globalRoomTemp || boilerCurrTemp < 30)
    {
      if (targetHCTemp != currentTargetHCTemp)
      {
        targetHCTemp = currentTargetHCTemp;
        ot.setBoilerTemperature(targetHCTemp);
      }
    }
    else if (targetRoomTemp < globalRoomTemp && boilerCurrTemp >= 53)
    {
      if (targetHCTemp != 1)
      {
        targetHCTemp = 1;
        ot.setBoilerTemperature(targetHCTemp);
      }
    }

    unsigned long resp1 = ot.setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling, enableOTCCompensation);

    isFlame = ot.isFlameOn(resp1);

    HCTemp = ot.getBoilerTemperature();

    modulation = ot.getModulation();

    uint16_t resp2 = ot.sendRequest(ot.buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::MaxCapacityMinModLevel, 0));
    maxCapacity = (resp2 >> 8) & 0xff;
    minModulation = (uint8_t)resp2 & 0xff;

    resp1 = ot.sendRequest(ot.buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::Toutside, 0));
    outside = ot.getFloat(resp1);

    // data = ot.temperatureToData(22);
    // boilerRoomTemp = globalRoomTemp;
    // setRoomTemp = ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA, OpenThermMessageID::Tr, data));
    // msgTypeRoomTemp = (setRoomTemp << 1) >> 29;

    resp2 = ot.sendRequest(ot.buildRequest(OpenThermMessageType::READ_DATA, OpenThermMessageID::ASFflags, 0));
    gasCrash = (resp2 >> 8) & 0xff;

    gasCrashFlags = (uint8_t)resp2;
    crashRaw = resp2;
    // gasCrash = ot.getFault();
  }
  else
  {
    initOpenTherm();
  }
}

//------------------------------------------------------------------------------------------------//

void handleNotFound()
{
  HttpServer.send(404, "text/plain", "404: Not found");
}

void handlePause()
{
  String result = "Succeed! " + !is_pause;
  HttpServer.send(200, "text/html", result);
  is_pause = !is_pause;
  delay(1000);
}

void handleReboot()
{
  String result = "Succeed! Now rebooting...";
  HttpServer.send(200, "text/html", result);
  delay(1000);
  ESP.restart();
}

void handleGetMainStatus()
{
  JSONVar json;
  byte ar[6];
  WiFi.macAddress(ar);
  char macAddr[18];
  sprintf(macAddr, "%2X:%2X:%2X:%2X:%2X:%2X", ar[0], ar[1], ar[2], ar[3], ar[4], ar[5]);
  json["text"] = "in progress...";
  json["IP"] = WiFi.localIP().toString();
  json["MAC"] = macAddr;
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
  json["targetHCTemp"] = targetHCTemp;
  json["currentTargetHCTemp"] = currentTargetHCTemp;
  json["boiler"] = responseStatus;
  timeClient.update();
  json["Time"] = timeClient.getFormattedTime();
  HttpServer.send(200, "application/json", JSON.stringify(json));
  delay(1000);
}

void handleSetTargetRoomTemp()
{
  if (HttpServer.hasArg("plain") == false)
  {
  }
  String body = HttpServer.arg("plain");

  JSONVar json = JSON.parse(body);

  targetRoomTemp = json["targetRoomTemp"];
  currentTargetHCTemp = json["currentTargetHCTemp"];

  EEPROM.begin(EEPROMSize);
  EEPROM.put(addrTargetRoomTemp, targetRoomTemp);
  EEPROM.put(addrCurrentTargetHCTemp, currentTargetHCTemp);
  EEPROM.commit();
  EEPROM.end();

  HttpServer.send(200, "text/html", "vars is set");
}

int AnalogRead(int pin, int iter = 50, int timeout = 1)
{
  unsigned int val = 0;
  for (int i = 0; i < iter; i++)
  {
    val += analogRead(pin);
    delay(timeout);
  }

  val = val / iter;
  return val;
}

int AnalogReadRaw(adc1_channel_t pin, int iter = 50, int timeout = 1)
{
  uint64_t val = 0;
  for (int i = 0; i < iter; i++)
  {
    val += adc1_get_raw(pin);
    delay(timeout);
  }

  return val / iter;
}

int AnalogReadAvg(adc1_channel_t pin, int iter = 50, int timeout = 1)
{
  uint64_t val = 0;
  for (int i = 0; i < iter; i++)
  {
    val += adc1_get_raw(pin);
    delay(timeout);
  }
  return val / iter;
}

int AnalogReadMax(adc1_channel_t pin, int iter = 50, int timeout = 1)
{
  uint64_t val = 0;
  for (int i = 0; i < iter; i++)
  {
    uint64_t curr = adc1_get_raw(pin);
    if (curr > val)
      val = curr;
    delay(timeout);
  }

  return val;
}

int AnalogReadTrueRMS(adc1_channel_t pin, int iter = 50, int timeout = 1)
{
  uint64_t val = 0;
  for (int i = 0; i < iter; i++)
  {
    uint64_t curr = adc1_get_raw(pin);
    // if(curr>val)
    val += curr * curr;
    delay(timeout);
  }

  return sqrt(val / iter);
}

int AnalogReadExtAdsTemp(int ended)
{
  uint64_t val = 0;
  for (int i = 0; i < 50; i++)
  {

    val += ads_temp.readADC_SingleEnded(ended);
    delay(1);
  }
  return val / 50;
}

double Thermister(int val)
{

  DEBUG_MSGLN(val);
  double V_NTC = (double)val * 0.000125; //* 0.000805860806;  // /1023
  DEBUG_MSGLN(V_NTC);
  double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
  DEBUG_MSGLN(R_NTC);
  R_NTC = log(R_NTC);
  double Temp = 1 / (kA + (kB + (kC * R_NTC * R_NTC)) * R_NTC);
  Temp = Temp - 273.15;
  return Temp;
}

double boilerThermister(int val)
{

  DEBUG_MSGLN(val);
  double V_NTC = (double)val * 0.000125; //* 0.000805860806;  // /1023
  DEBUG_MSGLN(V_NTC);
  double R_NTC = (Rs * V_NTC) / (Vcc - V_NTC);
  DEBUG_MSGLN(R_NTC);
  R_NTC = log(R_NTC);
  double Temp = 1 / (bkA + (bkB + (bkC * R_NTC * R_NTC)) * R_NTC);
  Temp = Temp - 273.15;
  return Temp;
}

void IRAM_ATTR setFlag()
{
  portENTER_CRITICAL(&timerMux);
  run_now = true;
  portEXIT_CRITICAL(&timerMux);
}

void collectAndSend()
{
  String tempHome = getVoltageData();
  String tempOutput =  getSensorData(dht1, "Котельная:");;
  String tempStreet = getSensorData(dht2, "Улица:");
  String tempInput = getHeatCarrierData();
  String tempBoiler = getBoilerData();

  String tempFloors;
  tempFloors += getFloorData("http://192.168.0.11/data", "Подвал:");
  tempFloors += getFloorData("http://192.168.0.16/data", "Первый этаж, кухня:", true);
  tempFloors += getFloorData("http://192.168.0.10/data", "Второй этаж, спальня большая:");

  WiFiClientSecure httpsClient;
  httpsClient.setCACert(root_ca);

  if (httpsClient.connect(telegramAddress, 443)){

    String postData, Link;

    timeClient.update();

    Link = "https://" + String(telegramAddress) + "/bot" + botToken;
    postData = "chat_id=" + chat_id;

    if (messageId == 0){
      Link += "/SendMessage";
      postData += "&disable_notification=1";
    }else{
      Link += "/editMessageText";
      postData += "&message_id=" + String(messageId);
    }
    postData += "&parse_mode=HTML&text=" +
                tempHome + "-------\r\n" +
                tempStreet + "-------\r\n" +
                tempFloors + "-------\r\n" +
                tempOutput + "-------\r\n" +
                tempInput + "-------\r\n" +
                tempBoiler + "-------\r\n\r\n" +
                String("Время запуска запуска: <b>") + time_d_start + "</b>\r\n" +
                String("Обновлено в: ") + timeClient.getFormattedTime() + "\r\n" +
                "<i>heap: " + String(ESP.getFreeHeap()) + "</i>\r\n";

    String req = String("GET ") + Link + " HTTP/1.1\r\n" +
                 "Host: " + telegramAddress + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
                 "Content-Length: " + postData.length() + "\r\n" +
                 "\r\n" + // Connection: close\r\n
                 postData;
    telegramLastMessage = req;
    httpsClient.println(req);

    String line;
    while (httpsClient.connected()){
      line = httpsClient.readStringUntil('\n');
      if (line == "\r"){
        // Serial.println("headers received");
        break;
      }
    }

    DEBUG_MSGLN("reply was:");
    DEBUG_MSGLN("==========");

    while (httpsClient.available()){
      line = httpsClient.readStringUntil('\n'); // Read Line by Line
      DEBUG_MSGLN(line);                        // Print response
    }

    DEBUG_MSGLN("==========");
    DEBUG_MSGLN("\r\n");

    telegramResponse = line;
    JSONVar reply_json = JSON.parse(line);

    if (JSON.typeof(reply_json) != "undefined" && (bool)reply_json["ok"] == true){
      if (messageId == 0){
        messageId = (int)reply_json["result"]["message_id"];
        EEPROM.begin(EEPROMSize);
        EEPROM.put(addrMessageId, messageId);
        EEPROM.commit();
        EEPROM.end();
        DEBUG_MSGLN(messageId);
      }
    }else{
      messageId = 0;
      char *buf = new char[64];
      telegramStatus = httpsClient.lastError(buf, sizeof(buf));
      delete buf;
      DEBUG_MSGLN("Connection failed!");
    }
  }

  httpsClient.flush();
  httpsClient.stop();
}

String getHeatCarrierData(){
  double outCurrTemp = (double)round(Thermister(AnalogReadExtAdsTemp(0)) * 10) / 10;
  double onCurrTemp = (double)round(Thermister(AnalogReadExtAdsTemp(2)) * 10) / 10;

  boilerCurrTemp = (double)round(boilerThermister(AnalogReadExtAdsTemp(3)) * 10) / 10;

  String smile;
  if (outCurrTemp > lastTemp)
    smile = "\xF0\x9F\x94\xA5";
  else if (outCurrTemp < lastTemp)
    smile = "\xE2\x9D\x84";
  else
    smile = "";
  lastTemp = outCurrTemp;

  String curr_ten = "вкл.";

  return String("Система:\r\n") +
         String("подача: <b>") + outCurrTemp + "°C</b> " + smile + "\r\n" +
         String("обратка: <b>") + onCurrTemp + "°C</b>\r\n" +
         String("бойлер: <b>") + boilerCurrTemp + "°C</b>\r\n";
}

String getSensorData(DHT &dh, String title){
  float h = dh.readHumidity();
  float t = dh.readTemperature();
  float i = dh.computeHeatIndex(h, t, false);

  String tempStreet;
  return title + "\r\n" +
         String("Влажность: <b>") + h + "%</b>\r\n" +
         String("Температура: <b>") + t + "°C</b>\r\n" +
         String("Тепловой индекс: <b>") + i + "°C\\t</b>\r\n";
}

String getVoltageData(){
  double L1 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_7, 1000) * 0.000805860806;;
  double L2 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_4, 1000) * 0.000805860806;;
  double L3 = (double)AnalogReadTrueRMS(ADC1_CHANNEL_6, 1000) * 0.000805860806;;

  return "Дом:\r\n" +
             String("Фаза 1: <b>") + (int)round(L1 * (double)kL1) + "V</b>\r\n" +
             String("Фаза 2: <b>") + (int)round(L2 * (double)kL2) + "V</b>\r\n" +
             String("Фаза 3: <b>") + (int)round(L3 * (double)kL3) + "V</b>\r\n";
}

String getBoilerData(){
  String boiler;
portENTER_CRITICAL(&openThermTimerMux);
  boiler = String("Котёл:") + (isFlame ? " \xF0\x9F\x94\xA5" : "") + "\r\n" +
           String("комната: <b>") + globalRoomTemp + "°C</b>\r\n" +
           String("комната, задано: <b>") + targetRoomTemp + "°C</b>\r\n" +
           String("улица: <b>") + outside + "°C</b>\r\n" +
           String("подача: <b>") + HCTemp + "°C</b> " + "\r\n" +
           String("подача, задано: <b>") + targetHCTemp + "°C</b> " + "\r\n" +
           String("модуляция пламени: <b>") + modulation + "%</b>\r\n" +
           String("минимальный уровень модуляции: <b>") + minModulation + "%</b>\r\n" +
           String("максимальная мощность котла: <b>") + maxCapacity + "Kw</b>\r\n" +
           String("Отопление: <b>") + enableCentralHeating + "</b>\r\n" +
           String("Охлаждение: <b>") + enableCooling + "</b>\r\n" +
           String("Горячая вода: <b>") + enableHotWater + "</b>\r\n";
  if (gasCrash){
    boiler += "\xE2\x9B\x94" + String("<b>ВНИМАНИЕ! авария на газовом котле. Код: ") + String(gasCrash) + "</b>\r\n" +
                               String("флаги: ") + String(gasCrashFlags) + "\r\n" +
                               String("сообщение: ") + String(crashRaw) + "\r\n";
  }
  portEXIT_CRITICAL(&openThermTimerMux);
  return boiler;
}

String getFloorData(String url, String title){
  return getFloorData(url, title, false);
}

String getFloorData(String url, String title, boolean setGlobal){
  String floor;
  if (http.begin(url))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK){
      String payload = http.getString();
      JSONVar floorData = JSON.parse(payload);
      floor = +"-------\r\n" +
                    title+"\r\n" +
                    String("Температура: <b>") + String((double)floorData["temperature"]) + "°C</b>\r\n" +
                    String("Влажность: <b>") + String((double)floorData["humidity"]) + "%</b>\r\n" +
                    String("Тепловой индекс: <b>") + String((double)floorData["heatIndex"]) + "°C/t</b>\r\n";
      if(setGlobal){
         globalRoomTemp = (double)floorData["temperature"];
      }
    }else{
      floor = String("Error, code: ") + String((int)httpCode) + "\r\n";
    }
    http.end();
  }
  return floor;
}

void loop()
{

  ArduinoOTA.handle();
  HttpServer.handleClient();
  if (communicateNow)
  {
    communicateBoiler();
    portENTER_CRITICAL(&openThermTimerMux);
    communicateNow = false;
    portEXIT_CRITICAL(&openThermTimerMux);
  }

  if (run_now && !is_pause)
  {
    digitalWrite(LED, HIGH);
    unsigned int msecLast = millis();
    collectAndSend();
    unsigned int nowmsec = millis();
    lastExec = (nowmsec - msecLast) / 1000;
    DEBUG_MSGLN(lastExec);
    DEBUG_MSGLN(String(ESP.getFreeHeap()));
    digitalWrite(LED, LOW);

    portENTER_CRITICAL(&timerMux);
    run_now = false;
    portEXIT_CRITICAL(&timerMux);
    timerWrite(timer, 0);
    timerAlarmEnable(timer);

    while (Serial2.available() > 0)
    { // если есть доступные данные
      // считываем байт
      String incomingByte = Serial2.readString();
      DEBUG_MSGLN("UART available:");
      DEBUG_MSGLN(incomingByte);
    }
    if (Serial.available() > 0)
    { // если есть доступные данные
      // считываем байт
      String incomingByte1 = Serial.readString();
      DEBUG_MSGLN("Send to Serial2:");
      DEBUG_MSGLN(incomingByte1);
      Serial2.println(incomingByte1);
    }
  }
}

void initNetwork()
{
  DEBUG_MSGLN("Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  if (WiFi.status() == WL_CONNECTED)
  {
    DEBUG_MSGLN("Connected");
  }
  else
  {
    DEBUG_MSGLN("Connect error!");
  }
}

void initHTTPServer()
{
  HttpServer.onNotFound(handleNotFound);
  HttpServer.on("/", handleGetMainStatus);
  HttpServer.on("/reboot", handleReboot);
  HttpServer.on("/pause", handlePause);

  HttpServer.on("/set_mess_id0", []()
                {
                        messageId = 0;
                        String result = "Succeed! ";
                        HttpServer.send(200, "text/html", result);
                        delay(1000); });

  HttpServer.on("/set-room-temp", HTTP_POST, handleSetTargetRoomTemp);

  HttpServer.on(OTA_PATH, HTTP_GET, []()
                { HttpServer.send(200, "text/html", loginIndex); });
  HttpServer.on("/serverIndex", HTTP_GET, []()
                { HttpServer.send(200, "text/html", serverIndex); });
  /*handling uploading firmware file */
  HttpServer.on("/update", HTTP_POST, []()
                {
        HttpServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart(); }, []()
                {
        HTTPUpload &upload = HttpServer.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        } });

  HttpServer.begin();
}

void initOTA()
{
  ArduinoOTA
      .onStart([]()
               {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

void initEEPROM()
{
  int tempMessageId = 0;
  double tempTargetRoomTemp = 0;
  double tempCurrentTargetHCTemp = 0;

  EEPROM.begin(EEPROMSize);
  EEPROM.get(addrMessageId, tempMessageId);
  EEPROM.get(addrTargetRoomTemp, tempTargetRoomTemp);
  EEPROM.get(addrCurrentTargetHCTemp, currentTargetHCTemp);
  DEBUG_MSGLN(tempMessageId);
  if (tempMessageId > 0)
  {
    messageId = tempMessageId;
  }
  if (tempTargetRoomTemp > 0)
  {
    targetRoomTemp = tempTargetRoomTemp;
  }
  if (tempCurrentTargetHCTemp > 0)
  {
    currentTargetHCTemp = tempCurrentTargetHCTemp;
  }
}

void initADC()
{
  ads_temp.setGain(GAIN_ONE);
  ads_temp.begin(0x48);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11); // 39
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // 34
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11); // 35
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); // 32
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11); // 33
}

void initTimeClient()
{
  timeClient.begin();
  timeClient.update();
  unsigned long epochTime = 0;
  uint ticker = 0;
  while (epochTime < 1670000000 || ticker < 15)
  {
    timeClient.update();
    epochTime = timeClient.getEpochTime();
    DEBUG_MSGLN("Getting actual NTP time ...");
    delay(2000);
    ticker++;
  }

  struct tm *ptm = gmtime((time_t *)&epochTime);

  int currentYear = ptm->tm_year + 1900;
  int currentMonth = ptm->tm_mon + 1;
  int monthDay = ptm->tm_mday;

  time_d_start = String(monthDay) + "." + String(currentMonth) + "." + String(currentYear) + " " + timeClient.getFormattedTime();
}

void initOpenTherm()
{
  ot.begin(handleInterrupt);

  unsigned long response = ot.setBoilerStatus(false, false, false, enableOTCCompensation);
  responseStatus = ot.getLastResponseStatus();
  if (responseStatus == OpenThermResponseStatus::SUCCESS)
  {
    DEBUG_MSGLN("Central Heating: " + String(ot.isCentralHeatingActive(response) ? "on" : "off"));
    DEBUG_MSGLN("Hot Water: " + String(ot.isHotWaterActive(response) ? "on" : "off"));
    DEBUG_MSGLN("Flame: " + String(ot.isFlameOn(response) ? "on" : "off"));

    ot.setBoilerTemperature(targetHCTemp);
    // setHCTemp = ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA, OpenThermMessageID::TSet, data));
    // msgTypeHCTemp = (setHCTemp << 1) >> 29;
    // ot.sendRequest(ot.buildRequest(OpenThermMessageType::WRITE_DATA, OpenThermMessageID::TrSet,targetRoomTemp));
  }
  if (responseStatus == OpenThermResponseStatus::NONE)
  {
    DEBUG_MSGLN("Error: OpenTherm is not initialized");
  }
  else if (responseStatus == OpenThermResponseStatus::INVALID)
  {
    DEBUG_MSGLN("Error: Invalid response " + String(response, HEX));
  }
  else if (responseStatus == OpenThermResponseStatus::TIMEOUT)
  {
    DEBUG_MSGLN("Error: Response timeout");
  }
}

void initTimers()
{
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &setFlag, true);
  timerAlarmWrite(timer, 100000, false);
  timerAlarmEnable(timer);

  openThermTimer = timerBegin(1, 8000, true);
  timerAttachInterrupt(openThermTimer, &startCommunicate, true);
  timerAlarmWrite(openThermTimer, 10000, true);
  timerAlarmEnable(openThermTimer);
}

void setup()
{

  dht1.begin();
  dht2.begin();

  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(1000);

  DEBUG(Serial.setDebugOutput(true));
  DEBUG_MSGLN("Start init device...");

  initEEPROM();
  initNetwork();

  if (!MDNS.begin(host))
  {
    DEBUG_MSGLN("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  http.setConnectTimeout(3000);
  http.setTimeout(3000);
  initHTTPServer();
  initOTA();
  initADC();
  initTimeClient();
  initOpenTherm();
  initTimers();
}

int main()
{
  init();
  initVariant();
  delay(1);
#if defined(USBCON)
  USBDevice.attach();
#endif
  setup();
  for (;;)
  {
    loop();
  }
  return 0;
                  }