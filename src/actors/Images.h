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

#include <log4ino/Log.h>
#include <main4ino/Actor.h>
#include <main4ino/Boolean.h>
#include <main4ino/Buffer.h>
#include <main4ino/Integer.h>
#include <main4ino/Misc.h>
#include <utils/Hexer.h>

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
  Metadata *md;
  uint8_t *imagesBin[NRO_IMGS];
  Buffer *imagesHex[NRO_IMGS];

public:
  Images(const char *n) {
    name = n;
    for (int i = 0; i < NRO_IMGS; i++) {
      imagesBin[i] = new uint8_t[IMG_SIZE_BYTES];
      imagesHex[i] = new Buffer(IMG_SIZE_BYTES * 2);
      for (int j = 0; j < IMG_SIZE_BYTES; j++) {
        imagesBin[i][j] = 0;
      }
    }
    md = new Metadata(n);
  }

  const char *getName() {
    return name;
  }

  Act act(Metadata *md) {
    return Act("");
  }

  CmdExecStatus command(Cmd *) {
    return NotFound;
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
      setPropValue(m, targetValue, actualValue, imagesHex[i]);
      if (m == SetCustomValue) {
        Hexer::hexToByte((uint8_t *)imagesBin[i], imagesHex[i]->getBuffer());
      }
    }
    if (m != GetValue) {
      getMetadata()->changed();
    }
  }

  int getNroProps() {
    return ImagesConfigStateDelimiter;
  }

  Metadata *getMetadata() {
    return md;
  }

  uint8_t *get(int i) {
    return imagesBin[POSIT(i) % NRO_IMGS];
  }
};

#endif // IMAGES_INC
