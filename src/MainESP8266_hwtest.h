void hwTest() {

  Buffer aux(32);
  log(CLASS_MAIN, Debug, "USER INFO");
  aux.fill("VER: %s", STRINGIFY(PROJ_VERSION));
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("NAM: %s", DEVICE_NAME);
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  aux.fill("ID : %d", ESP.getChipId());
  m.getNotifier()->message(0, 2, aux.getBuffer());
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);
  log(CLASS_MAIN, Debug, "HW test");

#ifdef HARDWARE_TEST
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);
  lcdImg('l', NULL);

  log(CLASS_MAIN, Debug, "..Face test");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  lcdImg('c', initImage);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..R. arm up");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 3, 100);
  log(CLASS_MAIN, Debug, "..Left arm up");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(3, 3, 100);
  log(CLASS_MAIN, Debug, "..Arms down");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  arms(0, 0, 100);
  log(CLASS_MAIN, Debug, "..Red led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', true);
  log(CLASS_MAIN, Debug, "..Red led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', false);
  log(CLASS_MAIN, Debug, "..Y. led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('y', true);
  log(CLASS_MAIN, Debug, "..Y. led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('y', false);
  log(CLASS_MAIN, Debug, "..W. led on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('w', true);
  log(CLASS_MAIN, Debug, "..W. led off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('w', false);
  log(CLASS_MAIN, Debug, "..Fan on");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('f', true);
  log(CLASS_MAIN, Debug, "..Fan off");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('f', false);
  log(CLASS_MAIN, Debug, "..Random %lu %lu %lu", random(10000), random(10000), random(10000));
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..SPIFFS");
  SPIFFS.begin();
  File f = SPIFFS.open("/version.txt", "r");
  if (!f) {
    log(CLASS_MAIN, Warn, "File reading failed");
  } else {
    String s = f.readString();
    log(CLASS_MAIN, Info, "File content: %s", s.c_str());
  }
  SPIFFS.end();
  delay(HARDWARE_TEST_STEP_DELAY_MS);

#endif

  ios('r', false);
  ios('y', false);
  ios('w', false);
  ios('f', false);
  lcdImg('l', NULL);
}