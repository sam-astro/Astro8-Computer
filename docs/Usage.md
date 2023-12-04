---
category: Getting Started
nav_order : 4
layout : default
title : Usage
---

# Usage

## Emulator
The program called `astro8` serves multiple purposes.
* Emulates machine code just like the real hardware
* Compiles Armstrong into assembly
* Assembles assembly into machine code and stores it into an `.AEXE` binary file for easy redistribution

To run your code or a program, type `astro8` followed by the path of the file:

```
> astro8 ./main.asm
```
OR

```
> astro8 ./main.armstrong
```

{: .tip .caps}
The type of file will be automatically determined as Armstrong, Assembly, or AstroEXE. You don't need to specify which one it is, unless you are trying to *only* compile **without** running it.

You may also provide extra options like so:
```
Usage: astro8 [options] <path>
```

List of options:
| **_Option_** | **_Description_** |
|----|---|
|`-h, --help`|               Display this help menu|
|`--version`|                Display the current version|
|`-c, --compile`|            Only compile and assemble Armstrong code to .ASM. Will not start emulator.|
|`-a, --assemble`|           Only assemble assembly code into AEXE. Will not start emulator.|
|`-r, --run`|                Run an already assembled program in AstroEXE format (program.AEXE)|
|`-nk, --nokeyboard`|        Disable the keyboard input|
|`-wb, --webcam`|            Enable webcam (uses default, only works on Windows)|
|`-nm, --nomouse`|           Disable the mouse input|
|`-v, --verbose`|            Write extra data to console for better debugging|
|`-vv, --superverbose`|      Write a lot of extra data to console for even better debugging|
|`-cm, --classicmode`|       Run Emulator in classic mode, using slow but realistic microcode instead of high performance|
|`-f, --freq <value>`|       Override the default CPU target frequency with your own. <br/> Default = 16    higher = faster <br/> {: .note .caps}High frequencies may be too hard to reach for some cpus|
|`--imagemode [frames]`|     Don't render anything, instead capture [frames] number of frames, which is 10 by default, and save to disk|


## Logisim
Along with the emulator, you can look at the actual circuit design for the system and even run your programs in it. 

{: .note .caps}
Currently a work in progress due to recent major upgrades

1. Compile/Assemble your program using `Astro8-Emulator`. It will save the machine code to a file called `program_machine_code` automatically. This file is located directly next to the `Astro8-Emulator` executable.
2. Open the file called `cpu-circuit.circ` in the newest version of [**Logisim Evolution**](https://github.com/logisim-evolution/logisim-evolution/releases)
3. Locate the RAM area, and find the one called `MEMORY`
4. Right-click on it, and click `Load Image...`
5. In the file view that just appeared, locate your compiled program file directly next to the `Astro8-Emulator` executable, and click `Open`
6. Press play, and it should run.
