#define HWSERIAL Serial1

#include "utils.h"
#include "usb_keyboard.c"

#define KBD_BUFFSZ 200
#define PREFIX 17 // CTRL-Q

char inChar;
char kbd_buff[KBD_BUFFSZ];
int kbd_idx = 0;

int in_mode = 1;
bool switch_mode = false;


void setup() {
  HWSERIAL.begin(115200);
  delay(1000);
}

void loop() {
  if (HWSERIAL.available() > 0) {
    inChar = HWSERIAL.read();

    if (inChar == PREFIX) {
      switch_mode = true;
      SerialPrintf("Select Mode: ");
      return;
    }
    if (switch_mode) {
      if (inChar >= 49 && inChar <= 51 ) {
        in_mode = inChar - 48;
      } else {
        in_mode = 3;
      }
      SerialClear();
      SerialPrintfln("Switching to mode %i", in_mode);
      switch_mode = false;
      return;
    }


    switch(in_mode) {
      case 1: //COMMAND MODE
        if (inChar == '\n' || inChar == '\r' || kbd_idx >= KBD_BUFFSZ-1) {
          SerialPrintfln("");
          kbd_buff[kbd_idx++] = '\0';
          parse(kbd_buff);
          kbd_idx = 0;
        } else {
          //TODO: React to special characters (backspace, arrow keys...)
          kbd_buff[kbd_idx++] = inChar;
          HWSERIAL.write(inChar);
        }
        break;

      case 2: //INTERACTIVE MODE
        interactive_echo(inChar);
        break;

      case 3: //DEBUG MODE
        SerialPrintfln("Recv -> Character: %c - ASCII-Code: %3i - USB-Scancode: %3i", inChar, inChar, unicode_to_keycode(inChar));
        //SerialPrintfln("Recv -> ASCII-Code:: %3i", inChar);
        break;
    }
  }
}

void interactive_echo(char str) {
  //TODO: Print on Serial
  //TODO: Make it work on Windows
  char key;
  if (str <= 26) {
    Keyboard.set_modifier(MODIFIERKEY_CTRL);
    Keyboard.set_key1(str+3);
    Keyboard.send_now();
    Keyboard.set_modifier(0);
    Keyboard.set_key1(0);
    Keyboard.send_now();
  } else if (str == 27) {
    if (HWSERIAL.read() == 91) {
      key = HWSERIAL.read();
      if (65 <= key <= 68) {
        if (key == 65)
          key = KEY_UP;
        if (key == 66)
          key = KEY_DOWN;
        if (key == 67)
          key = KEY_RIGHT;
        if (key == 68)
          key = KEY_LEFT;
        Keyboard.set_key1(key);
        Keyboard.send_now();
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
      }
    }
  } else {
    Keyboard.print(str);
  }

  /* switch(str) {
    case 3:
      Keyboard.set_modifier(MODIFIERKEY_CTRL);
      Keyboard.set_key1(6);
      HWSERIAL.write("^C\r\n");
      Keyboard.send_now();
      break;
    case 4:
      Keyboard.set_modifier(MODIFIERKEY_CTRL);
      Keyboard.set_key1(KEY_D);
      HWSERIAL.write("^D\r\n");
      Keyboard.send_now();
      break;
    case 8:
      Keyboard.set_modifier(MODIFIERKEY_CTRL);
      Keyboard.set_key1(KEY_H);
      Keyboard.send_now();
      break;
    case 13:
      Keyboard.set_modifier(MODIFIERKEY_CTRL);
      Keyboard.set_key1(KEY_M);
      Keyboard.send_now();
      break;
    case 37:
      Keyboard.set_key1(KEY_LEFT);
      Keyboard.send_now();
      break;
    default:
      Keyboard.print(str);
      break;
  } */
}

void parse(char * str) {
}
