#include "usb-keyboard.h"

char in_ascii;
char kbd_buff[KBD_BUFFSZ];
int kbd_idx = 0;
int crs_idx = 0;

int mode = 1;
int newmode = 0;
enum mode {INVALID, COMMAND, INTERACTIVE, DEBUG};
const char* mode_strings[] = {"invalid", "command", "interactive", "debug"};
const char* selectMode = "Select Inputmode: [1] Command - [2] Interactive - [3] Debug";
volatile bool key_pressing = false;

void setup() {
  HWSERIAL.begin(57600);
  delay(1000);
  SerialClear();
  SerialPrintfln("Switching to %s mode", mode_strings[mode]);
}

void clear_serial_buffer() {
  while(HWSERIAL.available() > 0) {
    HWSERIAL.read();
  }
}

void loop() {
  // if (HWSERIAL.available() > 0) {
  if ((!key_pressing) && HWSERIAL.available() > 0) {
    in_ascii = HWSERIAL.read();
    // SerialPrintfln("re");

    if (in_ascii<0 || in_ascii>127)
      // Ignore non-basic ascii characters
      return;

    if ((newmode = mode_select(in_ascii, mode))) {
      mode = newmode;
      return;
    }

    switch(mode) {
      case COMMAND:
        command_mode(in_ascii);
        break;

      case INTERACTIVE:
        interactive_mode(in_ascii);
        break;

      case DEBUG:
        debug_mode(in_ascii);
        break;
    }
  }
}
