#ifndef APP_H
#define APP_H

#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <OpenTherm.h>
#include <Update.h>
#include <WebSerial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <driver/adc.h>
#include <map>
#include <vector>
#include "DHT.h"
#include "soc/rtc_wdt.h"

// Внешняя функция для чтения температуры сенсора
extern "C" uint8_t temprature_sens_read();

// Определения пинов UART2
#define RXD2 16  // RXX2 pin
#define TXD2 17  // TX2 pin

// Макросы для отладки
#define DEBUG(x) \
   if (dev == 1) (x)
#define DEBUG_MSG(x)   \
   if (dev == 1) {     \
      Serial.print(x); \
   }
#define DEBUG_MSGLN(x)   \
   if (dev == 1) {       \
      Serial.println(x); \
   }

#include "init.h"

#if (dev == 1)
#define host "esp32_dev"
#else
#define host "esp32"
#endif

// Пины DHT
#define DHT_PIN1 16
#define DHT_PIN2 18

// Пины OpenTherm
const int inPin = 13;   // for Arduino, 4 for ESP8266 (D2), 21 for ESP32
const int outPin = 15;  // for Arduino, 5 for ESP8266 (D1), 22 for ESP32

// Пин светодиода
#define LED 2

// Порт сервера
#define SERVER_PORT 80

// Коэффициенты для вольтметра
const double kL1 = 158.693926;
const double kL2 = 158.397487;
const double kL3 = 159.856864;

// Коэффициенты для термистора в системе
const double kA = 0.00158933576950404;
const double kB = 0.000244017890892347;
const double kC = 0.000000213896311397919;
// Коэффициенты для термистора котла
const double bkA = 0.0016086108187479;
const double bkB = 0.000274663510997117;
const double bkC = 0.000000198074563227963;

// Адреса переменных в EEPROM
const uint addrMessageId = 0;
const uint addrTargetRoomTemp = addrMessageId + sizeof(int);
const uint addrCurrentTargetHCTemp = addrTargetRoomTemp + sizeof(double);
const uint addrBoilerMinTemp = addrCurrentTargetHCTemp + sizeof(double);
const uint addrBoilerMaxTemp = addrBoilerMinTemp + sizeof(double);
const uint EEPROMSize = addrBoilerMaxTemp;  // Общий размер

struct FloorDataConfig {
   String url;
   String label;
   bool is_kitchen = false;  // По умолчанию false, если не кухня

   // Конструктор для удобства инициализации
   FloorDataConfig(const String& u, const String& l, bool ik = false)
       : url(u), label(l), is_kitchen(ik) {}
};

// --- Объявление класса ---
class App {
  public:
   App();
   void handle();  // Основной метод обработки
   void init();    // Функция инициализации

   friend void setup();
   friend void loop();
   friend int app_main();
   friend void static_start_communicate();
   friend void static_set_flag();

  private:
   // --- Web Server ---
   AsyncWebServer HttpServer;

   // --- HTTP Client ---
   HTTPClient https;
   HTTPClient http;

   // Необходим для NTPClient, теперь объявлен глобально
   WiFiUDP ntpUDP;

   // --- Time Client ---
   NTPClient timeClient;

   // --- HTTPS Client ---
   WiFiClientSecure client;

   // Функции инициализации
   void initNetwork();
   void initHTTPServer();
   void initOTA();
   void initEEPROM();
   void initADC();
   void initTimeClient();
   void initOpenTherm();
   void initTimers();

   DHT dht1;
   DHT dht2;

   // --- Ticker ---
   hw_timer_t* timer;
   portMUX_TYPE timerMux;
   volatile bool run_now;
   volatile bool is_pause;

   // --- OpenTherm ---
   hw_timer_t* openThermTimer;
   portMUX_TYPE openThermTimerMux;
   OpenTherm ot;
   OpenThermResponseStatus responseStatus;
   bool communicateNow;

   // --- ADS ---
   Adafruit_ADS1115 ads_temp;

   // --- Variables ---
   int messageId;
   int telegramStatus;
   String telegramResponse;
   String telegramLastMessage;
   String time_d_start;

   // OpenTherm Variables
   double globalRoomTemp;
   double targetRoomTemp;
   double boilerCurrTemp;
   double boilerMinTemp;
   double boilerMaxTemp;
   bool enableCentralHeating;
   bool enableHotWater;
   bool enableCooling;
   bool enableOTCCompensation;
   uint8_t gasCrash;
   uint8_t gasCrashFlags;
   uint16_t crashRaw;
   double targetHCTemp;
   double currentTargetHCTemp;
   double HCTemp;
   float outside;
   bool isFlame;
   float modulation;
   uint8_t maxCapacity;
   uint8_t minModulation;

   // NTC Variables
   double lastTemp;
   unsigned int Rs;
   double Vcc;

   // ADC Variables
   double pin1, pin2, pin3, pin4, pin5, pin6;

   // --- Константы ---
   static const String TELEGRAM_MESSAGE_SEPARATOR;

   // Структура этажей
   std::vector<FloorDataConfig> floorConfigs;

   // --- Private Methods ---
   // OpenTherm
   void IRAM_ATTR startCommunicate();
   void communicateBoiler();

   // Web Server
   void handleNotFound(AsyncWebServerRequest* request);
   void handlePause(AsyncWebServerRequest* request);
   void handleReboot(AsyncWebServerRequest* request);
   void handleGetMainStatus(AsyncWebServerRequest* request);
   void handleSetTargetRoomTemp(AsyncWebServerRequest* request, uint8_t* data,
                                size_t len, size_t index, size_t total);
   void handleUpload(AsyncWebServerRequest* request, String filename,
                     size_t index, uint8_t* data, size_t len, bool final);

   // ADC
   int AnalogRead(int pin, int iter = 50, int timeout = 1);
   int AnalogReadRaw(adc1_channel_t pin, int iter = 50, int timeout = 1);
   int AnalogReadAvg(adc1_channel_t pin, int iter = 50, int timeout = 1);
   int AnalogReadMax(adc1_channel_t pin, int iter = 50, int timeout = 1);
   int AnalogReadTrueRMS(adc1_channel_t pin, int iter = 50, int timeout = 1);
   int AnalogReadExtAdsTemp(int ended);

   // NTC
   double Thermister(int val);
   double boilerThermister(int val);

   // Ticker
   void IRAM_ATTR setFlag();

   // Telegram
   void collectData(String& tempOutput, String& tempStreet, String& tempInput,
                    String& tempBoiler, String& tempFloors);
   String buildTelegramMessageText(const String& tempOutput,
                                   const String& tempStreet,
                                   const String& tempInput,
                                   const String& tempBoiler,
                                   const String& tempFloors);
   String buildTelegramRequest(const String& messageText);
   String sendTelegramRequest(const String& request);
   void processTelegramResponse(const String& response);
   void collectAndSend();
   String getHeatCarrierData();
   String getSensorData(DHT& dh, String title);
   String getVoltageData();
   String getBoilerData();
   String getFloorData(String url, String title);
   String getFloorData(String url, String title, boolean setGlobal);

   // Indication
   void one_blink();
   void two_blink();
   void three_blink();

   // EEPROM
   void saveMessageIdToEEPROM();

   // --- Метрики времени выполнения ---
   std::map<String, unsigned long> execution_times;
   void recordExecutionTime(const String& methodName, unsigned long startTime);
};

// --- Статические функции для ISR ---
void IRAM_ATTR static_start_communicate();
void IRAM_ATTR static_set_flag();

#endif  // APP_H