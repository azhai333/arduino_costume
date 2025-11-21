# Wearable LED Arcade Costume

This is the code for a Halloween costume I built that turns a hoodie into a fully playable arcade machine. It uses a LED matrix attached to the chest to display video games, with LED strips running along the rest of the hoodie for additional visual effects.

## How it Works
The system runs on an ESP32 and controls a 16x32 NeoPixel matrix. I wrote the following games completely from scratch:

## Games Included
*   **Tetris:** Full implementation with piece rotation and line clearing.
*   **Flappy Bird:** Gravity-based jumping and pipe generation.
*   **Space Invaders:** Player shooting, enemy movement patterns, and barriers.
*   **Etch-A-Sketch:** A drawing mode that uses the rotary encoders to draw lines on the matrix.

## Hardware
*   **Microcontroller:** ESP32 (See pinout at the top of the source file)
*   **Display:** 16x32 Flexible NeoPixel (WS2812B) Matrix
*   **Lighting:** NeoPixel Strips (approx. 800 LEDs)
*   **Controls:**
    *   Analog Joystick (Movement)
    *   4x Push Buttons (A, B, X, Y)
    *   2x Rotary Encoders (Left and Right shoulders)

## Dependencies
To run this, you will need the following Arduino libraries installed:
*   `Adafruit_NeoPixel`
*   `Adafruit_GFX`
*   `Adafruit_NeoMatrix`
*   `Encoder`

## Setup
1.  Wire up the controls and matrix according to the pin configuration comment at the top of the source file.
2.  Install the required libraries in the Arduino IDE.
3.  Upload the code to the board.
4.  Use the joystick/buttons to select a game from the main menu.
