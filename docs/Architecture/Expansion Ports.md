---
layout : default
parent : Architecture
title : Expansion Ports
category: Architecture
nav_order : 5
---

# Expansion Ports

{: .note .caps}
There are currently 3 hardware expansions built into the emulator. There are plans to allow for custom ones but that hasn't been implemented yet.

## Types

| **_Type_**     | **_Description_**                                           |
|----------------|-------------------------------------------------------------|
| **Keyboard**   | Get keyboard input                                          |
| **Mouse**      | Get mouse position and keys                                 |
| **Sound Card** | Pause and play specific frequencies of the 4 sound channels |


### Keyboard
The keyboard expansion detects key presses from the keyboard. The keycode is sent using the lower 8 bits (0bXXXXXXXX). They are automatically converted by hardware from ASCII to the SDCII format before reaching the expansion port.

### Mouse
The mouse expansion port holds multiple pieces of data:

```
binary:  R L XXXXXXX YYYYYYY
```

| **_Symbol_** | **_Data_**             |
|--------------|------------------------|
| **R**        | Right mouse button bit |
| **L**        | Left mouse button bit  |
| **X**        | X coordinates          |
| **Y**        | Y coordinates          |

You should process the data before using it, by bit shifting and bitwise AND-ing the numbers. The following example uses C for simplicity, but can very easily be translated to Yabal, and with a bit more effort, Armstrong. The variable titled `mouseVal` is assumed to be the expansion port value of the mouse.

```c
bool rightMousePressed = (mouseVal >> 15) & 1;  // Shift value from the left side to the right and get it by itself
bool leftMousePressed = (mouseVal >> 14) & 1;  // Shift value from the left side to the right and get it by itself

int mouseXPos = (mouseVal >> 7) & 0b1111111;  // Shift X coord value from the left side to the right and get it by itself
int mouseYPos = mouseVal & 0b1111111;  // Get y coord by itself
```

### Sound Card
The sound card is a 4 channel audio output device. It has squarewave A, squarewave B, triangle wave, and noise. The binary data encoding looks like this:

```
binary:  FFFFF CCC
```

| **_Symbol_** | **_Data_**               |
|--------------|--------------------------|
| **F**        | Target frequency integer |
| **C**        | Index of channel to edit |

The frequency is a value between 0 and 32. The channel is a value 0 through 4.

To change the frequency of a channel, send a value to the expansion port like: `0b00001001`, which would set the second channel (squarewave B) to play at a frequency of 1 (not in Hz, see conversion table). If a channel is not playing when you change it's frequency, it will begin playing. To stop the playback of a channel, send a second value to the expansion port with a frequency of 0, like: `0b00000001` which would turn off the second channel.


{: .note .caps}
Due to only 5 bits of precision, frequencies are approximated, and you can [see the conversion table](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Expansion%20Ports.html#conversion-table) to compare values sent to the expansion port with their corresponding actual frequencies.

#### Conversion table

| **_Sound Card Index_** | **_Actual Frequency Output_** | **_Music Note_** | **_Octave_** |
|---|---|---|---|
| 0 | N/A| N/A | N/A |
| 1 | 33| C | 1 |
| 2 | 37| D | 1 |
| 3 | 41| E | 1 |
| 4 | 55| A | 1 |
| 5 | 62| B | 1 |
| 6 | 65| C | 2 |
| 7 | 69| C# | 2 |
| 8 | 73| D | 2 |
| 9 | 78| D# | 2 |
| 10 | 82| E | 2 |
| 11 | 87| F | 2 |
| 12 | 93| F# | 2 |
| 13 | 98| G | 2 |
| 14 | 104| G# | 2 |
| 15 | 110| A | 2 |
| 16 | 117| A# | 2 |
| 17 | 123| B | 2 |
| 18 | 131| C | 3 |
| 19 | 139| C# | 3 |
| 20 | 147| D | 3 |
| 21 | 156| D# | 3 |
| 22 | 165| E | 3 |
| 23 | 175| F | 3 |
| 24 | 185| F# | 3 |
| 25 | 196| G | 3 |
| 26 | 207| G# | 3 |
| 27 | 220| A | 3 |
| 28 | 233| A# | 3 |
| 29 | 247| B | 3 |
| 30 | 262| C | 4 |
| 31 | 294| D | 4 |
