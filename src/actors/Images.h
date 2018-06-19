#ifndef IMAGES_INC
#define IMAGES_INC

#include <main4ino/Misc.h>
#include <Hexer.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Value.h>

#define CLASS_IMAGE "IM"

#define IMG_SIZE_BYTES 16
#define NRO_IMGS 4

enum ImagesConfigState {
  ImagesConfigImg0 = 0, // image in hexadecimal format
  ImagesConfigImg1,
  ImagesConfigImg2,
  ImagesConfigImg3,
  ImagesConfigStateDelimiter // delimiter of the configuration states
};

class Images : public Actor {

private:
  const char *name;
  Timing timing;
  uint8_t *images[NRO_IMGS];

public:
  Images(const char *n) : timing(Never) {
    name = n;
    for (int i = 0; i < NRO_IMGS; i++) {
      images[i] = new uint8_t[IMG_SIZE_BYTES];
      for (int j = 0; j < IMG_SIZE_BYTES; j++) {
        images[i][j] = 0;
      }
    }
  }

  const char *getName() {
    return name;
  }


  void act() {

  }

  const char *getPropName(int propIndex) {
    switch (propIndex) {
      case (ImagesConfigImg0):
        return "im0"; // image 0 (for any routine)
      case (ImagesConfigImg1):
        return "im1";
      case (ImagesConfigImg2):
        return "im2";
      case (ImagesConfigImg3):
        return "im3";
      default:
        return "";
    }
  }

  void getSetPropValue(int propIndex, GetSetMode m, const Value *targetValue, Value *actualValue) {
    if (propIndex >= ImagesConfigImg0 && propIndex < (NRO_IMGS + ImagesConfigImg0)) {
      int i = (int)propIndex - (int)ImagesConfigImg0;
      if (m == SetCustomValue) {
        Buffer<IMG_SIZE_BYTES * 2> target(targetValue); // 2 chars per actual bitmap byte
        Hexer::hexToByte((uint8_t *)images[i], target.getBuffer(), MINIM((strlen(target.getBuffer())), (IMG_SIZE_BYTES * 2)));
      }
      if (actualValue != NULL) {
        actualValue->load("<*>");
      }
    } else {
      switch (propIndex) {
        default:
          break;
      }
    }
  }

  int getNroProps() {
    return ImagesConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Timing *getFrequencyConfiguration() {
    return &timing;
  }

  uint8_t* get(int i) {
    return images[POSIT(i) % NRO_IMGS];
  }

};

#endif // IMAGES_INC
