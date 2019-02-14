void hwTest() {

#define HARDWARE_TEST_STEP_DELAY_MS 3000

#ifdef HARDWARE_TEST
  log(CLASS_MAIN, Debug, "USER INFO");
  m.getNotifier()->message(0, 2, "VER: %s", STRINGIFY(PROJ_VERSION));
  log(CLASS_MAIN, Debug, aux.getBuffer());
  delay(USER_DELAY_MS);

  delay(HARDWARE_TEST_STEP_DELAY_MS);
  ios('r', IO_OFF);
  ios('y', IO_OFF);
  ios('w', IO_OFF);
  ios('f', IO_OFF);
  lcdImg('l', NULL);
  log(CLASS_MAIN, Debug, "..BEGINS");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Arms -> down");
  arms(0, 0, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Right arm up");
  arms(0, 9, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Left arm up");
  arms(9, 0, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Lcd white");
  lcdImg('w', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Lcd dim");
  lcdImg('_', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Lcd bright");
  lcdImg('-', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Lcd black");
  lcdImg('-', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Red led on");
  ios('r', IO_ON);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Red led off");
  ios('r', IO_OFF);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Y. led on");
  ios('y', IO_ON);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Y. led off");
  ios('y', IO_OFF);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..W. led on");
  ios('w', IO_ON);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..W. led off");
  ios('w', IO_OFF);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Fan on");
  ios('f', IO_ON);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Fan off");
  ios('f', IO_OFF);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  log(CLASS_MAIN, Debug, "..Random %lu %lu %lu", random(10000), random(10000), random(10000));
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m.command("move Fs.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Nhello.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move A99.A00.B99.Z.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move D0.D1.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move D2.D3.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move D/.D\.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Du.Dn.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Mc3.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Fs.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move FS.W2.Fs.");
  m.command("move Lry.W2.Lrn.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Mq2.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
  m.command("move Mp2.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

#endif
}
