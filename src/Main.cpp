/**
 * This file contains:
 * - entry point for arduino programs (setup and loop functions)
 * - declaration of HW specific functions (the definition is in a dedicated file)
 * - other functions that are not defined by HW specific but that use them, that are required by the module
 *   (so that it can be passed as callback).
 * The rest should be put in Module so that they can be tested regardless of the HW used behind.
 */

#include <Main.h>

ModuleBotino *m;

#ifdef ARDUINO

#ifdef ESP8266 // on ESP8266
#include <MainESP8266.h>
#endif // ESP8266

#ifdef ESP32 // on ESP8266
#include <MainESP32.h>
#endif // ESP8266

#else // on PC
#include <MainX86_64.h>
#endif // ARDUINO

#define WIFI_CONNECTION_RETRIES 6

#define VERSION_DESCRIPTION_JSON "{"\
  "\"version\":\"" STRINGIFY(PROJ_VERSION) "\","\
  "\"json\":["\
    "{"\
      "\"patterns\": [\"^.*.freq$\"],"\
      "\"descriptions\": [\"Frequency at which the actor will act and perform its duty. See <a href=\\\"https://bitbucket.org/mauriciojost/main4ino-arduino/src/master/README.md\\\" target=\\\"_blank\\\">here</a>.\"],"\
      "\"examples\": ["\
        "\"never     -> Never\","\
        "\"~10s  -> Every 10 seconds\","\
        "\"~30m  -> Every 30 minutes\","\
        "\"@00h00   -> Everyday at 00h00\","\
        "\"@08h00   -> Everyday at 8h\","\
        "\"[SMtwtfs]@15h00 -> At 15h00 on Sundays and Mondays\","\
        "\"[smTWtfs]~30mf6t18 -> Every 30 minutes from 06h until 18h on Tuesdays and Wednesdays\","\
        "\"oneoff    -> Once and go back to previous configuration\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^body.r\\\\d{1,2}$\"],"\
      "\"descriptions\": [\"Routine i-th of the body (format: frequency:pose1.pose2...poseN. . See <a href=\\\"https://github.com/mauriciojost/botino-arduino/blob/master/src/actors/Body.md\\\" target=\\\"_blank\\\">all pose codes here</a>.\"],"\
      "\"examples\": ["\
        "\"@12h00:M2Message at midday.A99. -> Tell message Lunch! at 12 every day and put arms up quickly\","\
        "\"never:D0. -> Dance 0 never (to disable the routine)\","\
        "\"oneoff:D0. -> Dance 0 just once\","\
        "\"@20h00:Lry.W2.Lrn. -> Turn on RED led at 20h00 for 2 seconds and turn it off\","\
        "\"~30m:M1Drink water!. -> Tell to Drink water! every 30 minutes\","\
        "\"[smTWtfs]~30mf6t18:M1Water!. -> Tell Water! with font size 1 every 30 minutes from 6 until 18 on Tuesdays and Wednesdays\","\
        "\"[SMtwtfs]@15h00:NBaby%eats!. -> Notify Baby eats! at 15h00 on Sundays and Mondays\","\
        "\"~10s:L?. -> Turn randomly all leds every 10 seconds\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^clocksync..zone$\"],"\
      "\"descriptions\": [\"Zone to pick up the current date-time from. See <a href=\\\"https://timezonedb.com/\\\" target=\\\"_blank\\\">here</a>.\"],"\
      "\"examples\": ["\
        "\"Europe/Paris\","\
        "\"America/Argentina\","\
        "\"Asia/Shanghai\","\
        "\"Africa/Freetown\""\
      "]"\
      "},"\
      "{"\
      "\"patterns\": [\"^notifier.n\\\\d{1,2}\"],"\
      "\"descriptions\": [\"Notification item i-th.\"],"\
      "\"examples\": [\"Drink water\"]"\
    "},"\
    "{"\
      "\"patterns\": [\"^commands.cm\\\\d{1,2}\"],"\
      "\"descriptions\": [\"Command i-th: name of the command, : character, and command to execute in i-th turn while pressing button.\"],"\
      "\"examples\": ["\
        "\"Arms up:move C99.\","\
        "\"Update:update\","\
        "\"Execute IFTTT event:move Ievent.\","\
        "\"Acknowledge notification: ack\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^images.im\\\\d{1,2}\"],"\
    	"\"descriptions\": [\"Bitmap of custom image i-th, represented in base 64. See <a href=\\\"https://docs.google.com/spreadsheets/d/1jXa9mFxeiN_bUji_WiCPKO_gB6pxQUeQ5QxgoSINqdc/edit\\\" target=\\\"_blank\\\">here</a>.\"],"\
      "\"examples\": ["\
        "\"00002814100828140000000003C00000 -> dead\","\
        "\"1FF820044A524A52481247E220041FF8 -> big smile\","\
        "\"1108081001803A40025C018008201110 -> sun\","\
        "\"448021000C1CD3A21441480197FE2200 -> cloudy\","\
        "\"00000000001C03A20441080107FE0000 -> clouds\","\
        "\"07E008100FF0381028103810081007E0 -> mug\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^propsync..forcesyncfreq$\"],"\
      "\"descriptions\": [\"Frequency at which the device performs a full synchronization with the server (required as the server performs regular cleanup on old records).\"],"\
      "\"examples\": [\"~72h\"]"\
    "},"\
    "{"\
      "\"patterns\": [\"^settings..lcdlogs$\"],"\
      "\"descriptions\": [\"Flag telling if the LCD shows the logs or not\"],"\
      "\"examples\": ["\
      "\"true -> logs in LCD on\","\
      "\"false -> logs in LCD off\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^settings..mperiodms$\"],"\
      "\"descriptions\": [\"User response time in ms. The lower this value the more power consumption.\"],"\
      "\"examples\": ["\
        "\"1000 -> 1 second of response timelogs in LCD on\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^settings..periodms$\"],"\
      "\"descriptions\": [\"Period (in ms) of wakeup.\"],"\
      "\"examples\": ["\
        "\"10000 -> every 10 seconds\","\
        "\"60000 -> every 60 seconds\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^settings..btnrout$\"],"\
      "\"descriptions\": [\"Determines which first moves will be executed randomly on button press. For instance, if set to 2, only moves 0 and 1 will be executed randomly. \"],"\
      "\"examples\": ["\
        "\"0\","\
        "\"1\","\
        "\"2\","\
        "\"3\""\
      "]"\
    "},"\
    "{"\
      "\"patterns\": [\"^.*.debug\"],"\
      "\"descriptions\": [\"Flag to enable/disable debug mode\"],"\
      "\"examples\": ["\
        "\"true -> debug enabled\","\
        "\"false -> debug disabled\""\
      "]"\
    "}"\
  "]"\
"}"


bool initWifiSimple() {
  Settings *s = m->getModuleSettings();
  log(CLASS_MAIN, Info, "W.steady");
  bool connected = initWifi(s->getSsid(), s->getPass(), true, WIFI_CONNECTION_RETRIES);
  return connected;
}

void setup() {
  m = new ModuleBotino();
  m->setup(setupArchitecture,
           lcdImg,
           arms,
           messageFunc,
           ios,
           initWifiSimple,
           stopWifi,
           httpPost,
           httpGet,
           clearDevice,
           readFile,
           writeFile,
           sleepInterruptable,
           deepSleepNotInterruptable,
           configureModeArchitecture,
           runModeArchitecture,
           commandArchitecture,
           infoArchitecture,
           updateFirmware,
           testArchitecture,
           apiDeviceLogin,
           apiDevicePass);
  ModuleStartupPropertiesCode ec = m->startupProperties();
  m->getModule()->setDescription(VERSION_DESCRIPTION_JSON);
  if (ec != ModuleStartupPropertiesCodeSuccess) {
    log(CLASS_MAIN, Error, "Failure: %d", (int)ec);
    abort("Could not startup");
  }
}

void loop() {
  m->loop();
}
