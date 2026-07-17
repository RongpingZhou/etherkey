#include "usb-keyboard.h"

// Util functions
int mode_select(char in_ascii, int oldmode) {
  int newmode = 0;
  char peekChar;

  if (in_ascii == PREFIX) {
    SerialPrintf("%s", selectMode);
    do {
      while (HWSERIAL.available() == 0) {
        delay(100);
      }
      peekChar = HWSERIAL.peek();
      if (peekChar == PREFIX) {
        in_ascii = HWSERIAL.read();
        SerialDeleteChars(strlen(selectMode));
        return 0;
      } else if (peekChar > '0' && peekChar < ('0'+sizeof(mode_strings)/sizeof(char*))) {
        in_ascii = HWSERIAL.read();
        newmode = in_ascii - '0';
        SerialClear();
        SerialPrintfln("Switching to %s mode", mode_strings[newmode]);
        return newmode;
      } else if (peekChar == 27) {
        in_ascii = HWSERIAL.read();
        SerialDeleteChars(strlen(selectMode));
        return oldmode;
      } else {
        in_ascii = HWSERIAL.read();
        SerialDeleteChars(strlen(selectMode));
        SerialPrintf("%s", selectMode);
      }
    } while (peekChar != 27 || peekChar != PREFIX || (peekChar >= 49 && peekChar <= 51));
  }
  return 0;
}

uint16_t escape_sequence_to_keycode(char key) {
  char in_ascii = key;
  uint16_t keycode = 0;

  if (in_ascii == 91) {
    in_ascii = HWSERIAL.peek();
    if (in_ascii >= 65 && in_ascii <= 68) {
      HWSERIAL.read();
      if (in_ascii == 65)
        keycode = KEY_UP;
      if (in_ascii == 66)
        keycode = KEY_DOWN;
      if (in_ascii == 67)
        keycode = KEY_RIGHT;
      if (in_ascii == 68)
        keycode = KEY_LEFT;
    } else if (in_ascii == 51) {
      HWSERIAL.read();
      in_ascii = HWSERIAL.peek();
      if (in_ascii == 126) {
        HWSERIAL.read();
        keycode = KEY_DELETE;
      }
    }
  }
  return keycode;
}

uint16_t special_char_to_keycode(char in_ascii) {
  uint16_t keycode = 0;

  switch(in_ascii) {
    case 10:  //LF
    case 13:  //CR
      keycode = KEY_ENTER;
      break;
    case 8:   //BS
    case 127: //DEL
      keycode = KEY_BACKSPACE;
      break;
    case 9:   //HT
      keycode = KEY_TAB;
      break;
    case 27:
      in_ascii = HWSERIAL.peek();
      if(in_ascii == 255) {
        keycode = KEY_ESC;
      } else {
        HWSERIAL.read();
        keycode = escape_sequence_to_keycode(in_ascii);
      }
      break;
  }
  return keycode;
}

uint16_t keyname_to_keycode(const char* keyname) {
  uint16_t keycode = 0;
  char keyname_lower[KEYNAME_BUFFSZ];

#ifdef MYDEBUG
  SerialPrintfln("\tKeyname: %-15s -> %x", keyname, str2int(keyname));
#endif

  // keyname matching shall be case-insensitive
  for (uint8_t i = 0; i <= strlen(keyname); i++) {
    keyname_lower[i] = tolower(keyname[i]);
  }

  // F1 - F12
  for (uint8_t f = 1; f <= 12; f++) {\
    char fkey[4];
    snprintf(fkey, sizeof(fkey), "f%d", f);
    if (!strcmp(fkey, keyname_lower)) {
      keycode = KEY_F1+f-1;
      break;
    }
  }

  // Check for other special keynames, if none has been found yet
  if (!keycode) {
    switch (str2int(keyname_lower)) {
      case str2int("enter"):
        keycode = KEY_ENTER;
        break;
      case str2int("escape"):
      case str2int("esc"):
        keycode = KEY_ESC;
        break;
      case str2int("space"):
        keycode = KEY_SPACE;
        break;
      case str2int("tab"):
        keycode = KEY_TAB;
        break;
      case str2int("backspace"):
      case str2int("bs"):
        keycode = KEY_BACKSPACE;
        break;
      case str2int("delete"):
      case str2int("del"):
        keycode = KEY_DELETE;
        break;
      case str2int("insert"):
      case str2int("ins"):
        keycode = KEY_INSERT;
        break;
      case str2int("up"):
        keycode = KEY_UP;
        break;
      case str2int("down"):
        keycode = KEY_DOWN;
        break;
      case str2int("left"):
        keycode = KEY_LEFT;
        break;
      case str2int("right"):
        keycode = KEY_RIGHT;
        break;
      case str2int("home"):
        keycode = KEY_HOME;
        break;
      case str2int("end"):
        keycode = KEY_END;
        break;
      case str2int("pgup"):
        keycode = KEY_PAGE_UP;
        break;
      case str2int("pgdn"):
        keycode = KEY_PAGE_DOWN;
        break;
      case str2int("win"):
      case str2int("windows"):
        keycode = KEY_LEFT_GUI;
        break;
    }
  }
  return keycode;
}

void usb_send_key(uint16_t key, uint16_t mod=0) {
  // Uses Keyboard.press from the Teensyduino Core Library
  // key can be either a ascii char or a keycode
#ifdef MYDEBUG
  char key_b[33];
  itoa(key, key_b, 2);
  char mod_b[33];
  itoa(mod, mod_b, 2);
  SerialPrintfln("\tSendKey: %6i = %08s | mod: %6i = %08s", key, key_b, mod, mod_b);
#endif
  if (mod) Keyboard.press(mod);
  Keyboard.press(key);
}

// Interactive mode functions
void interactive_mode(char in_ascii) {
  uint16_t keycode;

  if ((keycode = special_char_to_keycode(in_ascii))) {
    usb_send_key(keycode);
    switch(keycode) {
      case KEY_ENTER:
        SerialPrintfln("");
        break;
      case KEY_BACKSPACE:
        SerialDeleteChars(1);
        break;
    }
  } else if (in_ascii <= 26) {
    in_ascii = in_ascii + 'a' - 1;
    usb_send_key(in_ascii, MODIFIERKEY_CTRL);
  } else {
    HWSERIAL.write(in_ascii);
    usb_send_key(in_ascii);
  }
}

// Command mode functions
void command_mode(char in_ascii) {
  uint16_t keycode = special_char_to_keycode(in_ascii);
  unsigned long start_time = micros();
  unsigned long elapse_time = micros() - start_time;
  // SerialPrintfln("\n\rkeycode: %d\n\r", keycode);

  if(keycode) {
    switch(keycode) {
      case KEY_ENTER:
        // start_time = micros();
        SerialPrintfln("");
        kbd_buff[kbd_idx] = '\0';
        // for(int i = 0; i < strlen(kbd_buff); i++){
        //   HWSERIAL.write(kbd_buff[i]);
        // }
        // char kbd_buff_copy[200];
        // strcpy(kbd_buff_copy, kbd_buff);

        // SerialPrintfln("kbd_buff: %s", kbd_buff);
        c_parse(kbd_buff);
        // elapse_time = micros() - start_time;
        // SerialPrintfln("elapse_time: %d", elapse_time);
        // memset(kbd_buff, 0, KBD_BUFFSZ);
        crs_idx = kbd_idx;
        kbd_idx = 0;
        break;
      case KEY_BACKSPACE:
        if (crs_idx == kbd_idx) {
          // TODO: Currently only supports deleting characters at the end of the line
          // To delete in the middle of the Array, all following characters must be moved
          SerialDeleteChars(1);
          if (kbd_idx>0) kbd_idx--;
          if (crs_idx>0) crs_idx--;
        }
        break;
      case KEY_LEFT:
        if (crs_idx>0) {
          SerialAnsiEsc("1D");
          crs_idx--;
        }
        break;
      case KEY_RIGHT:
        if (crs_idx<kbd_idx) {
          SerialAnsiEsc("1C");
          crs_idx++;
        }
        break;
      case KEY_UP:
        while (crs_idx>kbd_idx) {
          if (!kbd_buff[kbd_idx])
            kbd_buff[kbd_idx] = ' ';
          HWSERIAL.write(kbd_buff[kbd_idx++]);
        }
        break;
    }
  } else if (in_ascii == 3) {
    SerialClearLine();
    crs_idx = kbd_idx;
    kbd_idx = 0;
  } else if (kbd_idx >= KBD_BUFFSZ-1) {
    command_mode('\n');
  } else if (in_ascii <= 26) {
  } else {
    // HWSERIAL.write(in_ascii);
    if (crs_idx>kbd_idx) crs_idx = kbd_idx;
    kbd_buff[crs_idx++] = in_ascii;
    if (kbd_idx<crs_idx) kbd_idx++;
    // if(!key_pressing)
    // {
    //   HWSERIAL.write(in_ascii);
    //   if (crs_idx>kbd_idx) crs_idx = kbd_idx;
    //   kbd_buff[crs_idx++] = in_ascii;
    //   if (kbd_idx<crs_idx) kbd_idx++;
    // }
  }
}

void c_parse(char* str) {
  char* pch;

  key_pressing = true;

  if (!(pch = strtok(str," "))){
    key_pressing = false;
    return;
  }
  unsigned long start = micros();
#ifdef MYDEBUG
  SerialPrintfln("\tCommand: %-15s -> %x", pch, str2int(pch));
#endif
  switch (str2int(pch)) {
    case str2int("SendRaw"):
    case str2int("sendraw"):
      // Send the rest of the line literally
      if ((pch = strtok (NULL,"")))
        c_sendraw(pch);
      break;

    case str2int("Send"):
    case str2int("send"):
      // Send the rest of the line (and parse special characters)
      if ((pch = strtok (NULL,"")))
        c_send(pch);
      break;

    case str2int("Help"):
    case str2int("help"):
      // Display a informative help message
      //TODO
      break;

    default:
      // Check if input is a keyname and send it
      if (!c_parse_ext(pch, false, 0)) {
        // Show warning about invalid command
        //TODO
      }
      break;
  }
  if (key_pressing) {
    while (micros() - start <= (116240)) // mean value from the paper titiled "Observations on Typing from 136 Million Keystrokes"
    {
      clear_serial_buffer();
    }
  }
  key_pressing = false;
  Keyboard.releaseAll();
  SerialPrintf("Released\n");
}

bool c_parse_ext(char* str, bool send_single, int modifier) {
  // Sends matching keyname or reacts to special commands
  uint16_t key = 0;
  int num = 1;
  char* pch;

#ifdef MYDEBUG
  SerialPrintfln("\tCmd-ext: %-15s -> %x", str, str2int(str));
#endif
  if (!(key = keyname_to_keycode(str))) {
    if (send_single && !*(str+1) && (*str >= 32 || *str < 127)) {
      key = *str;
    }
  }
  if (key) {
    if ((str = strtok(NULL," ")))
      num = atoi(str);
    while (num>0) {
      usb_send_key(key, modifier);
      num--;
    }
    return true;
  } else {
    switch (str2int(str)) {
      case str2int("Sleep"):
      case str2int("sleep"):
      case str2int("Delay"):
      case str2int("delay"):
        // Sleep a certain amount of time in ms
        if ((pch = strtok(NULL,"")) && (num = atoi(pch))) {
          c_sleep(num);
          return true;

        }
        break;
      case str2int("UnicodeLinux"):
      case str2int("UCL"):
        // Send a single unicode character (on Linux)
        if ((pch = strtok (NULL,""))) {
          c_unicode(pch, true);
          return true;
        }
        break;

      case str2int("UnicodeWindows"):
      case str2int("UCW"):
        // Send a single unicode character (on Windows)
        if ((pch = strtok (NULL,""))) {
          c_unicode(pch, false);
        }
        break;
    }
  }
  return false;
}

void c_sendraw(char* pch) {
  char* c = pch;

  while (*c) {
    usb_send_key(*c);
    c++;
  }
}

void c_send(char* pch) {
  char* c = pch;
  int modifier = 0;
  bool braces = false;
  char keyname_buff[KEYNAME_BUFFSZ];
  int keyname_idx = 0;

  while (*c) {
    if (braces) {
      if ((keyname_idx >= KEYNAME_BUFFSZ-1) || *c == '}') {
        keyname_buff[keyname_idx] = '\0';
        if ((pch = strtok(keyname_buff," "))) {
          c_parse_ext(pch, true, modifier);
        }
        keyname_idx = 0;
        braces = false;
        modifier = 0;
      } else {
        keyname_buff[keyname_idx++] = *c;
      }
      c++;
      continue;
    }
    switch (*c) {
      case '!':
        // ALT
        modifier |= MODIFIERKEY_ALT;
        break;
      case '+':
        // SHIFT
        modifier |= MODIFIERKEY_SHIFT;
        break;
      case '^':
        // CTRL
        modifier |= MODIFIERKEY_CTRL;
        break;
      case '#':
        // WIN
        modifier |= MODIFIERKEY_GUI;
        break;
      case '{':
        braces = true;
        break;
      default:
        usb_send_key(*c, modifier);
        modifier = 0;
        break;
    }
    c++;
  }
  // @inproceedings{10.1145/3173574.3174220,
  // author = {Dhakal, Vivek and Feit, Anna Maria and Kristensson, Per Ola and Oulasvirta, Antti},
  // title = {Observations on Typing from 136 Million Keystrokes},
  // year = {2018},
  // isbn = {9781450356206},
  // publisher = {Association for Computing Machinery},
  // address = {New York, NY, USA},
  // url = {https://doi.org/10.1145/3173574.3174220},
  // doi = {10.1145/3173574.3174220},
  // abstract = {We report on typing behaviour and performance of 168,000 volunteers in an online study. 
  // The large dataset allows detailed statistical analyses of keystroking patterns, linking them to typing performance. 
  // Besides reporting distributions and confirming some earlier findings, we report two new findings. 
  // First, letter pairs that are typed by different hands or fingers are more predictive of typing speed than, 
  // for example, letter repetitions. Second, rollover-typing, wherein the next key is pressed before the previous one is released, 
  // is sur- prisingly prevalent. Notwithstanding considerable variation in typing patterns, 
  // unsupervised clustering using normalised inter-key intervals reveals that most users can be divided into 
  // eight groups of typists that differ in performance, accuracy, hand and finger usage, and rollover. 
  // The code and dataset are released for scientific use.},
  // booktitle = {Proceedings of the 2018 CHI Conference on Human Factors in Computing Systems},
  // pages = {1–12},
  // numpages = {12},
  // keywords = {text entry, modern typing behavior, large-scale study},
  // location = {Montreal QC, Canada},
  // series = {CHI '18}
  // }
  // delayMicroseconds(116240); // mean value from the paper titiled "Observations on Typing from 136 Million Keystrokes"
  // // delay(67); // 1000/60 = 66.6
  // // memset(kbd_buff, 0, KBD_BUFFSZ);
  // // memset(c, 0, KBD_BUFFSZ);
  // // memset(pth, 0, KBD_BUFFSZ);
  // Keyboard.releaseAll();
}

void c_unicode(char* pch, bool linux) {
  char* c = pch;
  int modifier = 0;
  if (linux) {
    modifier |= MODIFIERKEY_CTRL;
    modifier |= MODIFIERKEY_SHIFT;
    usb_send_key('u', modifier);
    while (*c) {
      usb_send_key(*c);
      c++;
    }
    usb_send_key(KEY_ENTER);
  } else {
    if ((!keyboard_leds) & (1<<0))
      usb_send_key(KEY_NUM_LOCK);
    Keyboard.press(MODIFIERKEY_ALT);
    Keyboard.press(KEYPAD_PLUS);
    Keyboard.release(KEYPAD_PLUS);
    while (*c) {
      if (*c >= '1' && *c <= '9') {
        Keyboard.press((uint16_t)(*c+40) | 0x4000);
        Keyboard.release((uint16_t)(*c+40) | 0x4000);
      } else if (*c == '0') {
        Keyboard.press(KEYPAD_0);
        Keyboard.release(KEYPAD_0);
      } else {
        Keyboard.press(*c);
        Keyboard.release(*c);
      }
      c++;
    }
    Keyboard.releaseAll();
  }
}

void c_sleep(int ms) {
#ifdef MYDEBUG
  SerialPrintfln("\tSleeping: %i ms", ms);
#endif
  delay(ms);
}


// Debug mode functions
void debug_mode(char in_ascii) {
  SerialPrintfln("ASCII: %3i = %c", in_ascii, in_ascii);
}
