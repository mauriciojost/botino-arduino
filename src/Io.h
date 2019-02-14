#ifndef IO_INC
#define IO_INC

enum IoMode { IoOff = 0, IoOn = 1, IoToggle = 2 };

IoMode invert(IoMode v) {
  if (v == IoOn) {
    return IoOff;
  } else if (v == IoOff) {
    return IoOn;
  } else {
    return v;
  }
}

#endif // IO_INC
