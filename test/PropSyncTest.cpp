#ifdef UNIT_TEST

// Auxiliary libraries
#include <unity.h>
#include <string.h>

// Being tested
#include <actors/sync/PropSync.h>
#include <main4ino/SerBot.h>
#include <main4ino/Array.h>

void setUp() {}

void tearDown() {}

bool initWifi() {
	return true; // always connected
}

int httpGet(const char* url, ParamStream* response) {
	if (strcmp("", url) == 0) {
		response->flush();
	}
  response->flush();
	return 1;
}

int httpPost(const char* url, const char* body, ParamStream* response) {
  response->flush();
	return 1;
}

void test_propsync_syncs_properties() {
	Clock clock("c");

	Array<Actor*> actors(1);
	actors.set(0, &clock);

	SerBot b(&clock, &actors);

  PropSync p("p");
  p.setBot(&b);

  p.setInitWifi(initWifi);
  p.setHttpGet(httpGet);
  p.setHttpPost(httpPost);

  //p.getFrequencyConfiguration()->setFrequency(OnceEvery1Second);

  p.act();

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_propsync_syncs_properties);
  return (UNITY_END());
}

#endif // UNIT_TEST
