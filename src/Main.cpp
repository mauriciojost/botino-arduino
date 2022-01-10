#include <Constants.h>
#include <ModuleBotino.h>
#include <Platform.h>
#include <main4ino/Misc.h>

#ifndef PROJ_VERSION
#define PROJ_VERSION "snapshot"
#endif // PROJ_VERSION

#define CLASS_MAIN "MA"

//////////////////////////////////////////////////////////////
// Provided by generic Main
//////////////////////////////////////////////////////////////

// Standard arduino setup


HttpResponse httpMethodCustom(HttpMethod m, const char *url, Stream *body, Table *headers, const char *fingerprint) {
  heartbeat();
  return httpMethod(m, url, body, headers, fingerprint);
}

std::function<bool ()> initWifiSimpleWrapper = []() {return initWifiSimple();};
std::function<void ()> stopWifiWrapper = []() {stopWifi();};

std::function<bool ()> needFirstSetup = [&]() { return wasHardwareReset(); };

void setup() {
  setupArchitecture();
  m = new ModuleBotino();
  m->setup(lcdImg,
           arms,
           messageFunc,
           ios,
           initWifiSimpleWrapper,
           stopWifiWrapper,
           httpMethodCustom,
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
           apiDevicePass,
           getLogBuffer,
           buttonIsPressed);
  m->getModule()->setNeedFirstSetupFunc(needFirstSetup);
  m->getModule()->setFirstSetupFunc(firstSetupArchitecture);

  log(CLASS_MAIN, Info, "Startup of properties");
#ifdef BIMBY_MODE
  StartupStatus c = m->startupPropertiesLight();
#else  // BIMBY_MODE
  StartupStatus c = m->startupProperties();
#endif // BIMBY_MODE
  m->getBot()->setMode(c.botMode);
  if (c.startupCode != ModuleStartupPropertiesCodeSuccess) {
    log(CLASS_MAIN, Error, "Failure: %d", (int)c.startupCode);
    bool i = sleepInterruptable(now(), 10);
    if (i) {
    } else {
      abort("Could not startup");
    }
  }
}

void loop() {
  m->loop();
}

#ifdef X86_64
#include <MavarduinoSimulatorLauncher.h>
#endif // X86_64
