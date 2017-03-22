#include <actors/ParamStream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLASS "ParamStream"

ParamStream::ParamStream() {
  commandsAvailable = 0;
  nroBytesReceived = 0;
}

size_t ParamStream::write(uint8_t b) {

  append(b);

  if (b == '&') {
    if (nroBytesReceived != 0) {
      Buffer<MAX_VALUE_STR_LENGTH> newValue;
      // Parse the potential command: c<configurable>p<property>=<value>
      char* beginStr = bytesReceived.getBuffer();
      char* indexStr = strtok(beginStr, "=&");
      char* valueStr = strtok(NULL, "=&");
      char* configStr = strtok(indexStr, "cp");
      char* propStr = strtok(NULL, "cp");

      printf("CONFIG: %s\n", configStr);
      printf("PROPER: %s\n", propStr);
      printf("VALUE : %s\n", valueStr);

      if (configStr != NULL && propStr != NULL && valueStr != NULL) {
        int confIndex = atoi(configStr);
        int propIndex = atoi(propStr);
        newValue.load(valueStr);
        addCommand(confIndex, propIndex, &newValue);
      }
    }
    nroBytesReceived = 0;
  }

  return 1;
}

void ParamStream::append(uint8_t b) {
  log(CLASS, Debug, "Byte: ", (int)b);
  nroBytesReceived = (nroBytesReceived + 1) % (MAX_VALUE_STR_LENGTH + 1);
  bytesReceived.getBuffer()[nroBytesReceived - 1] = b;
}

void ParamStream::addCommand(int confIndex, int propIndex, const Value* newValue) {
  if (commandsAvailable < MAX_NRO_COMMANDS) {
    commands[commandsAvailable].confIndex = confIndex;
    commands[commandsAvailable].propIndex = propIndex;
    commands[commandsAvailable].newValue.load(newValue);
    commandsAvailable++;
  }

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

int ParamStream::getNroCommandsAvailable() {
  return commandsAvailable;
}

Command* ParamStream::getCommands() {
  return commands;
}
