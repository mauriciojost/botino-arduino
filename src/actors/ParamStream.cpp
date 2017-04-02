#include <actors/ParamStream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLASS "ParamStream"

ParamStream::ParamStream() {
  commandsAvailable = 0;
}

size_t ParamStream::write(uint8_t b) {
  append(b);
  return 1;
}

void ParamStream::append(uint8_t b) {
  log(CLASS, Debug, "Byte: ", (int)b);
  bytesReceived.append(b);
}

int ParamStream::available() {
  // Not supported.
	return 0;
}
int ParamStream::read() {
  // Not supported.
	return -1;
}
int ParamStream::peek() {
  // Not supported.
	return -1;
}
void ParamStream::flush() {
  commandsAvailable = 0;
  bytesReceived.clear();
}
