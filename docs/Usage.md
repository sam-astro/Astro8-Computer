---
nav_order : 3
layout : default
title : Usage
---

# Usage

## Emulator
The file called `Astro8-Emulator` serves multiple purposes.
* Emulates machine code just like the real hardware
* Compiles Armstrong into assembly
* Assembles assembly into machine code and stores it into a file called `program_machine_code`

To run your code, you may either simply start the program where you will be prompted to input your code - OR, you can provide a path as a command line argument. You can either type directly into the command line (don't use any blank lines), or enter a path to your armstrong or assembly file and press enter ***twice***. The type will be determined by the first line of the file. ***All Armstrong files should have `#AS` as the first line.***

There is a second executable written in C# called `ResourceGenerator`. This is used to generate binary data from the character-set PNG file. Unless you want to change the font or add new characters, you don't need to use this.

## Logisim
Along with the emulator, you can look at the actual circuit design for the system and even run your programs in it. 
1. Compile/Assemble your program using `Astro8-Emulator`. It will save the machine code to a file called `program_machine_code` automatically. This file is located directly next to the `Astro8-Emulator` executable.
2. Open the file called `cpu-circuit.circ` in the newest version of [Logisim Evolution](https://github.com/logisim-evolution/logisim-evolution/releases)
3. Locate the RAM area, and find the one called `MEMORY`
4. Right-click on it, and click `Load Image...`
5. In the file view that just appeared, locate your compiled program file directly next to the `Astro8-Emulator` executable, and click `Open`
6. Press play, and it should run.