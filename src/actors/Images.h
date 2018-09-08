#ifndef IMAGES_INC
#define IMAGES_INC

/**
 * Images
 *
 * Holds custom images.
 *
 * The custom images respect a custom bitmap serialization, expressed in hexadecimal format.
 * To ease the design process, you can use the below link to create your own bitmap.
 *
 * [Image generator](https://docs.google.com/spreadsheets/d/1jXa9mFxeiN_bUji_WiCPKO_gB6pxQUeQ5QxgoSINqdc/edit#gid=0)
 *
 * Each image is IMG_SIZE_BYTES long (expressed in hexadecimal format), and this actor support NRO_IMGS images.
 *
 */

#include <Hexer.h>
#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <main4ino/Value.h>

#define CLASS_IMAGE "IM"

#define IMG_SIZE_BYTES 16
#define NRO_IMGS 4

enum ImagesConfigState {
  ImagesConfigImg0 = 0,      // string, image in hexadecimal format
  ImagesConfigImg1,          // string, image in hexadecimal format
  ImagesConfigImg2,          // string, image in hexadecimal format
  ImagesConfigImg3,          // string, image in hexadecimal format
  ImagesConfigStateDelimiter // delimiter of the configuration states
};

class Images : public Actor {

private:
  const char *name;
  Metadata* md;
  uint8_t *images[NRO_IMGS];

public:
  Images(const char *n) {
    name = n;
    for (int i = 0; i < NRO_IMGS; i++) {
      images[i] = new uint8_t[IMG_SIZE_BYTES];
      for (int j = 0; j < IMG_SIZE_BYTES; j++) {
        images[i][j] = 0;
      }
    }
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  void act() {}

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
    }
    if (m != GetValue) {
    	getMetadata()->changed();
    }
  }

  int getNroProps() {
    return ImagesConfigStateDelimiter;
  }

  void getInfo(int infoIndex, Buffer<MAX_EFF_STR_LENGTH> *info) {}

  int getNroInfos() {
    return 0;
  }

  Metadata *getMetadata() {
    return md;
  }

  uint8_t *get(int i) {
    return images[POSIT(i) % NRO_IMGS];
  }
};

#endif // IMAGES_INC
