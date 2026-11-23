# 3D LED Cube Snake Game

3D version of a classic Snake game, built for a 3-dimensional LED cube using Arduino and C++.

**Authors:** Stanisław Rybak and Jan Tchórzewski

## Features

* **Full 3D Movement:** Navigate the snake in all 3 dimensions using a custom 7-button controller.
* **Score Display:** The final score is rendered directly on the outer faces of the LED cube using a custom 3x5 font.
* **High Score Tracking:** Highest score is saved permanently in the Arduino's EEPROM memory.
* **Serial Logging:** Current score and high score are printed to the Serial Monitor.

## Hardware Requirements

* **Microcontroller:** Arduino Nano.
* **LEDs:** 125x WS2812B LEDs (5x5x5 cube) controlled via the Adafruit NeoPixel library.
* **Controller:** Custom pad with 7 push buttons (Up, Down, Left, Right, Forward, Backward, Reset).
* **Connection:** Ethernet twisted pair cable connecting the controller pad to the main Arduino board.

## 3D Printed Parts

The repository includes 3D models required to build the hardware:

* **Pad Casing:** Enclosure for the 7-button controller ('controller'). 
* **Soldering Tower:** A jig to help solder the LED cube layers accurately ('led soldering stand').
* **5x5 LED Base:** A grid to keep the bottom LEDs perfectly aligned ('led cube stand').
* **Arduino Nano Case:** Two-part enclosure for the microcontroller (`snake b` for the bottom part, `snakT` for the top part).

## Pin Configuration

The buttons use the internal pull-up resistors (`INPUT\_PULLUP`). Wire them between the specified pin and GND.

* **LED Data Pin:** D2
* **Reset Button:** D3
* **Up Button:** D4
* **Forward Button:** D5
* **Right Button:** D6
* **Left Button:** D7
* **Down Button:** D8
* **Backward Button:** D9

## Software Dependencies

* [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
* EEPROM (Arduino built-in)

## License

This project is licensed under the CC BY-NC 4.0 License.

<img width="1542" height="2047" alt="download" src="https://github.com/user-attachments/assets/cdc5c6ce-4aa4-4e5c-bec4-51e23d385a68" />
