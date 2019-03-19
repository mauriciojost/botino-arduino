void hwTest() {

#define HARDWARE_TEST_STEP_DELAY_MS 5000

  m->getNotifier()->message(0, 2, "VER: %s", STRINGIFY(PROJ_VERSION));
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Power off");
  ios('r', IoOff);
  ios('y', IoOff);
  ios('w', IoOff);
  ios('f', IoOff);
  lcdImg('l', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Arms -> down");
  arms(0, 0, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Right arm up");
  arms(0, 9, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Left arm up");
  arms(9, 0, 100);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Lcd white");
  lcdImg('w', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Lcd dim");
  lcdImg('_', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Lcd bright");
  lcdImg('-', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Lcd black");
  lcdImg('-', NULL);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Red led on");
  ios('r', IoOn);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Red led off");
  ios('r', IoOff);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Yellow led on");
  ios('y', IoOn);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Yellow led off");
  ios('y', IoOff);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "White led on");
  ios('w', IoOn);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "White led off");
  ios('w', IoOff);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Fan on");
  ios('f', IoOn);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Fan off");
  ios('f', IoOff);
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->getNotifier()->message(0, 1, "Random: %lu %lu %lu", random(10000), random(10000), random(10000));
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Fs.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Nhello.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move A99.A00.B99.Z.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move D0.D1.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move D2.D3.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move D/.D\\.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Du.Dn.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Mc3.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Fs.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move FS.W2.Fs.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Lry.W2.Lrn.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Mq2.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);

  m->command("move Mp2.");
  delay(HARDWARE_TEST_STEP_DELAY_MS);
}
