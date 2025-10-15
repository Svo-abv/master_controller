#include "App.h"  // Подключаем заголовочный файл, содержащий объявления

// --- Глобальный экземпляр приложения ---
extern App app;

// --- Основные функции Arduino ---
void setup() {

   rtc_wdt_protect_off();
   rtc_wdt_disable();

   Serial.begin(115200);
   Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

   DEBUG(Serial.setDebugOutput(true));
   DEBUG_MSGLN("Start init device...");

   pinMode(LED, OUTPUT);
   delay(1000);

   app.init();

   if (!MDNS.begin(host)) {
      DEBUG_MSGLN("Error setting up MDNS responder!");
      while (1) {
         delay(1000);
      }
   }
}

void loop() { app.handle(); }

int app_main() {
   init();
   initVariant();
   delay(1);
#if defined(USBCON)
   USBDevice.attach();
#endif
   setup();
   for (;;) {
      loop();
   }
   return 0;
}