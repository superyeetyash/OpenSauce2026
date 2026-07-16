# Color Quackdown

An interactive arcade reaction game built for OpenSauce 2026 based on the prompt "Make something interactive." Duck Hunt Reaction Arcade is a modern take on the classic carnival duck shooting game. Instead of shooting ducks, players test their reflexes, accuracy, and color recognition by hitting the correct button before time runs out. The goal was to create a project that could be understood instantly by spectators, encourage repeated play, and combine mechanical design, electronics, and embedded programming into one interactive experience.

## Gameplay

The game uses six mechanical ducks arranged in two rows of three. Each duck is mounted on a servo-powered rack-and-pinion lift that allows it to rise from behind a wall. Every duck has a fixed color. In front of the player are six arcade buttons arranged in the same 2×3 layout, each containing an RGB LED. At the beginning of every round, the button colors are randomized. When a duck rises, the player must identify its color and press the matching illuminated button before the timer reaches zero. If the correct button is pressed, the duck lowers, a success sound plays, the score increases, and the next round begins with less time available. If the wrong button is pressed or time expires, the game ends. The timer progressively decreases after each successful round with a minimum limit of 1.5 seconds.

## Features

The project includes six servo-powered rising duck targets, RGB LED arcade buttons, color recognition gameplay, increasing difficulty, score tracking, high score storage, arcade-style sound effects, a HUB75 RGB LED matrix display, and fully physical controls designed for public interaction at events.

## System Architecture

The Arduino Mega 2560 is the main controller and handles all real-time game functions, including button inputs, RGB LED control, servo movement, game timing, sound effects, and score management. A Raspberry Pi Pico 2 is used as a dedicated display controller for the HUB75 LED matrix. Since HUB75 panels require constant high-speed refreshing, separating the display system from the gameplay controller keeps the game responsive. The Mega communicates with the Pico 2 over UART, sending information such as score, timer values, and game state.

## Bill of Materials (BOM)

| Quantity | Component | Purpose |
|---|---|---|
| 1 | Arduino Mega 2560 | Main game controller handling inputs, outputs, and game logic |
| 1 | Raspberry Pi Pico 2 | Dedicated HUB75 LED matrix display controller |
| 1 | P5 HUB75 64×32 RGB LED Matrix | Main arcade display for score, timer, and animations |
| 6 | MG90S metal gear micro servos | Drives the duck rack-and-pinion lifting mechanisms |
| 6 | 4-pin arcade buttons | Player input controls |
| 6 | Common cathode RGB LEDs | Button lighting system |
| 18 | 220Ω resistors | Current limiting for RGB LED channels |
| 1 | Passive speaker or buzzer | Arcade sound effects |
| 1 | 5V 10A power supply | Powers servos, LED matrix, and electronics |
| 1 | Power distribution block | Splits 5V power between system components |
| 1 | 470–1000µF electrolytic capacitor | Reduces servo voltage spikes |
| 1 | Servo wiring harness or jumper wires | Connects servo power and signals |
| 1 | Assorted jumper wires | General electrical connections |
| 1 | Breadboard or custom PCB | Prototyping and connecting electronics |
| 1 | 3D printed duck assemblies | Physical duck targets |
| 6 | Rack-and-pinion lift assemblies | Converts servo rotation into vertical duck movement |
| 1 | Laser-cut or fabricated enclosure | Houses the arcade system |

## Pin Configuration

The RGB LEDs use a common cathode configuration, with the common pin connected to ground and each RGB channel connected to the Arduino Mega through a 220Ω resistor. The arcade buttons use one signal connection and one ground connection with `INPUT_PULLUP`, meaning a pressed button reads as LOW. The current pin assignments are: pins 2–13 for RGB channels, pins 22–27 for additional RGB channels, pins 28–33 for buttons, and pin 34 for the speaker. The complete pin configuration is stored at the top of `ColorMatchGame/ColorMatchGame.ino`.

## Power System

The servos and HUB75 display should not be powered directly from the Arduino Mega's 5V pin. A dedicated 5V power supply is used to power the system through a distribution block, with separate rails for the servo system and LED matrix. The Arduino Mega, Raspberry Pi Pico 2, servo power, and display power all share a common ground. A 470–1000µF capacitor is placed near the servo power rail to reduce voltage drops caused by servo startup current spikes.

Estimated power requirements:

| Component | Estimated Current |
|---|---|
| Arduino Mega 2560 | ~200mA |
| Six MG90S servos | Up to 2A+ during movement |
| HUB75 LED matrix | 2–3A depending on brightness |
| RGB LEDs and logic | ~300mA |

## Repository Structure

The repository contains the main Arduino firmware in `ColorMatchGame/ColorMatchGame.ino` and the mechanical CAD files in the `cad/` folder. The firmware contains the complete game logic, button handling, RGB control, servo control, timing system, sound effects, and EEPROM high score storage. The CAD files include the duck models, rack-and-pinion lift system, servo mounts, and housing components. The mechanical design is created using parametric OpenSCAD models, allowing dimensions to be modified easily.

## Running the Game

To run the game, open `ColorMatchGame/ColorMatchGame.ino` in the Arduino IDE, select the Arduino Mega or Mega 2560 board, upload the sketch, and open the Serial Monitor at 9600 baud. The firmware includes testing tools that can be accessed through Serial commands, including `t` for RGB LED testing and `c` for button calibration. During startup, the system checks for common wiring issues such as stuck buttons and incorrect LED connections.

## Mechanical Design

Each duck uses a servo-powered rack-and-pinion lift mechanism. A servo rotates a gear, which moves a rack vertically and raises the duck above the wall. The design includes approximately 120mm of vertical travel, 180° servo movement, six identical lift modules, 3D printed ducks, and laser-cut housing panels.

## Development Roadmap

Completed features include the RGB button color matching system, reaction game logic, sound effects, servo-controlled duck movement, rack-and-pinion CAD design, high score storage, HUB75 display support, and Mega-to-Pico UART communication.

## Project Goal

The goal of Duck Hunt Reaction Arcade was to create an arcade experience that requires no explanation. A spectator should immediately understand that a duck appears, a button lights up, the correct button must be pressed, and the score increases. The project combines mechanical engineering, electronics, embedded programming, and game design into a single interactive installation built for OpenSauce 2026.
## Pictures
<img width="3024" height="3024" alt="IMG_8560" src="https://github.com/user-attachments/assets/a1f3dd56-f906-4985-8864-4094a30a135f" />
<img width="3024" height="4032" alt="IMG_8562" src="https://github.com/user-attachments/assets/51169030-0185-404a-ae03-f9743e1fcf3f" />
<img width="3024" height="4032" alt="IMG_8565" src="https://github.com/user-attachments/assets/e07b2a31-da50-45e2-a050-8f1881bb390f" />
<img width="3024" height="4032" alt="IMG_8564" src="https://github.com/user-attachments/assets/190b87f1-5b0f-4bc2-9918-0c6a42ec60d2" />
<img width="3024" height="4032" alt="IMG_8563" src="https://github.com/user-attachments/assets/107aa109-ab1b-4236-9435-2b1e92b25231" />

