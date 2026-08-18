Etherkey
========
Emulate a conventional USB keyboard with a scriptable network capable microcontroller.

By using dedicated hardware it is possible to control systems even before the operating system is booted and without being dependent on the running software. For example this allows automatic bootloder selectios or modification of BIOS settings.

Version
-------
15/August/2026 by Rongping Zhou

Write the instructions to flash the Teensy board with 116 ms delay, this is to emulate 116 ms key press duration

9/July/2026 by Rongping Zhou

Write the instructions to flash the Teensy board with 67ms delay, this is to emulate 67ms key press duration

Requirements
------------
* [Teensy](https://www.pjrc.com/teensy/index.html) (tested on Teensy 3 and 4.1),
* One of the following:
  * a) USB-to-UART Adapter, [for example](http://www.adafruit.com/product/954)
  * b) [Raspberry PI](http://www.raspberrypi.org/)

Setup
-------
* Flash the Teensy with the sketch in the etherkey folder. (Using [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html))
* Connect the Teensy's USB-Port to the System you want to control.

### a) Flash the Teensy with the sketch in the etherkey folder.
* Prepare the computer, make sure that FUSE is installed on your system:

```shell
sudo add-apt-repository universe
sudo apt install libfuse2
```
* Refer to the below link for downloading and installing Arduino IDE 2:

https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing/

* Download Arduino IDE from the website:

https://www.arduino.cc/en/software/

* Before we can launch the editor, we need to first make it an executable file. This is done by:

    * right-click the file,
    * choose Properties,
    * select Permissions tab,
    * tick the Allow executing file as program box.

* To enable the Arduino IDE to access the serial port and upload code to your board, the following rule 
```shell
SUBSYSTEM=="usb", ATTR{idVendor}=="2341", ATTR{idProduct}=="0043", MODE="0666"
```

can be added to

```shell
/etc/udev/rules.d/99-arduino.rules
```

You can now double click the file to launch the Arduino IDE 2 on your Linux machine. 

* How to How to get Teensy 4.1 board

    Teensy 4.1 board can be bought from the local online store:

    https://core-electronics.com.au/teensy-4-1-headers.html

    or

    https://www.pjrc.com/store/


    Refer to the below website:

    https://www.pjrc.com/teensy/td_download.html

* Teensyduino

    To install Teensy on Arduino IDE 2.x, click File > Preferences (on MacOS, click Arduino IDE > Settings). On Windows 11 laptops with a small touchscreen display, you may need to scroll down to even if no scrollbar appears. In "Additional boards manager URLs", copy this link:

    https://www.pjrc.com/teensy/package_teensy_index.json

    In the main Arduino window, open Boards Manager by clicking the left-side board icon, search for "teensy", and click "Install".

    Download the Linux udev rules from https://www.pjrc.com/teensy/00-teensy.rules and copy the file to /etc/udev/rules.d.

```shell
sudo cp 00-teensy.rules /etc/udev/rules.d/
```

* Etherkey

    Etherkey is a Teensy 4.1 based USB HID device that can be used to send keystrokes to a computer. It is designed to be used with the Arduino IDE and can be programmed using the Arduino programming language.

    * Download the Etherkey code from the GitHub repository.

```shell
mkdir -p ~/hardware
cd ~/hardware
```

```shell
git clone https://github.com/RongpingZhou/etherkey.git
git checkout discrete_action/v1.6
```
to ensure to use 116240 microsecond (116.24 millisecond) key press duration in the tag of discrete_action/v1.6

* Flash the code
    * Open etherkey.ino
    * Select Teensy 4.1 hardware on Arduino IDE 2.x, click Tools > Board > Teensy (for Arduino 2.04 or later) > Teensy 4.1.
    * Select USB Type, click Tools > USB Type > Keyboard or select USB Type, click Tools > USB Type > Serial + Keyboard + Mouse + Joystick
    * Select port, click Tools > Port > /dev/hidraw5 RawHID (Teensy 4.1)
    * Then click Sketch > Verify/Compile to compile the code.
    * Click Sketch > Upload to upload the code to Teensy.
    * Teensy window pops up, press Button on Teensy to manually enter Program Mode.

### b) Direct connection to Teensy
You may connect directly to the Teensy, using a USB-to-UART Adapter or USB-to-RS232 cable.

Connect Ground to Teensy's GND-Pin, TX to Pin 0, RX to Pin 1.

When USB to UART cable or USB-to-RS232 cable connects to an Ubuntu machine, there has already been a /dev/ttyUSB0 directory there, delete it with the below command:

```shell
sudo rm -r /dev/ttyUSB0
```

Then connect the USB to UART cable or USB-to-RS232 cable to this computer, it should recognize the USB to UART serial device as /dev/ttyUSB0.

After last step, you can use the program for the paper titled "Measure Sim-to-Real Gap: Designing an Affordable Real-World Benchmark Platform for Reinforcement Learning in AIoT Systems" at the repository of https://github.com/RongpingZhou/real_world_program

Or you can use any tool you like to connect to the Teensy. Baudrate is 57600, device most likely `/dev/ttyUSB0` on Linux/UNIX. For example: `cu -l /dev/ttyUSB0 -s 57600`


### c) Using a Raspberry PI for Network features
Example setup with a Raspberry PI for the ethernet connection.

![](doc/teensy-pi_bb.png)

Connect GND of the Raspberry to Teensy's GND-Pin, TX to Pin 0, RX to Pin 1.


When using Raspbian as operating system, the serial port [must be configured for outgoing connections](http://elinux.org/RPi_Serial_Connection#Connection_to_a_microcontroller_or_other_peripheral).
After that a serial connection can be established with `cu -l /dev/ttyAMA0 -s 57600`. On newer Raspberry PI or Raspbian versions you might need to use `/dev/serial0` instead.


Usage and Syntax
-----

### Mode selection
`Ctrl+Q` to switch between modes.

Available modes:

* Interactive mode
* Command mode
* Debug mode

### Interactive mode
Directly sends the recieved keystroke
Supported Characters:

* All printable ASCII characters
* Arrow keys
* Backspace
* Enter
* Delete
* Tab
* Escape

### Command mode
Parses the whole line and interprets the first Word as command. Available commands:

#### SendRaw
Sends the rest of the line literally

#### Send
Sends the rest of the line while interpreting special characters.
This command behaves similarly to the send command of [AutoHotkey](http://ahkscript.org/docs/commands/Send.htm)


##### Modifiers
The following characters are treated as modifiers:

* `!`: Send the next character with the ALT key pressed.

  Example: `Send text!a` sends the keys "This is text" and then presses `ALT+a`.

* `+`: Send the next character with the SHIFT key pressed.

  Example: `Send +abC` sends the keys "AbC".

* `^`: Send the next character with the CTRL key pressed.

  Example: `Send ^c` sends a `CTRL+c` keystroke.

* `#`: Send the next character with the WIN key pressed.

  Example: `Send #d` sends a `WIN + d` keystroke.

These modifiers can be combined, so a `^+t` would send a `CTRL+SHIFT+t`, thus restoring the last tab in Firefox.

##### Keynames
Non printable characters can be sent by specifying the keyname enclosed in braces:

* `{Enter}`
* `{Escape}`
* `{Space}`
* `{Tab}`
* `{Backspace}/{BS}`
* `{Delete}/{Del}`
* `{Insert}/{Ins}`
* `{Up}`
* `{Down}`
* `{Left}`
* `{Right}`
* `{Home}`
* `{End}`
* `{PgUp}`
* `{PgDn}`
* `{Windows}/{Win}`
* `{F1}`..`{F12}`

These keynames can also be combined with modifiers. For example `Send ^!{Delete}` sends the `CTRL+SHIFT+Delete` combination.

##### Escape sequence

To send a single character literally, it can be enclosed in braces:

* `{x}`
* `{!}`
* `{+}`
* `{^}`
* `{#}`
* `{{}`
* `{}}`

This syntax can also be used to repeat a keystroke multiple times:

Enclose the character or keyname followed by a whitespace and the number of repetitions in braces.

* For example: `{x 10}` sends the x character 10 times and `{Enter 5}` presses the Enter key 5 times.

#### Unicode characters
Arbitrary Unicode characters can be sent by specifying the 4-digit character code after the operating system specific command:

##### UnicodeLinux or UCL
Initializes the GKT+/Qt Unicode Sequence and sends the following 4-digit hexadecimal Unicode Character.

* For example: `{UCL 00F8}`

##### UnicodeWindows or UCW (experimental)
Initializes the Windows Unicode Sequence and sends the following Unicode Character. Please note: Some Windows applications require 4-digit decimal Code (e.g. Wordpad, Chrome), some other require 4-digit hexadecimal Code (e.g. Notepad++, Firefox)

You might as well need to [change a Registry Setting](http://en.wikipedia.org/wiki/Unicode_input#In_Microsoft_Windows) on your Windows machine.

* For example: `{UCW 00F8}`/`{UCW 0248}`


### Debug mode
Displays information about the received character (ASCII code, USB keycode)


Client
-----
TODO

Known issues/ Caveats
-----

### Keyboard Layouts
Due to the nature of the emulation on a USB keyboard layer, the list of
available characters is determined by the keyboard layout on the target host.

Additionally only basic ASCII characters are transmitted correctly over the
serial interface.

Thus an English keyboard layout is recommended for conventional usage. This
is also configured by default in the Teensyduino development environment, but
can be changed under "Tools" -> "Keyboard Layout".

To transmit other keys:

- Sending keys by their name in command mode
- Mixing keyboard layouts for transmission to send keystrokes defined by their position on the keyboard

  By setting the desired keyboard layout on the target system while configuring
  an English keyboard layout on the Teensy and the controlling system, it is
  possible to send keystronkes independent of the possibility to transmit the associated character.
  This works as the pressed key is interpreted on the controlling system.
