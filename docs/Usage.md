---
nav_order : 3
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

The type of file will be automatically determined as Armstrong, Assembly, or AstroEXE, or you may specify it with the options below.

You may also provide extra options like so:
```
Usage: astro8 [options] <path>
```

List of options:
```
  -h, --help               Display the help menu
```
```
  -c, --compile            Only compile and assemble Armstrong code. Will not start emulator.
```
```
  -a, --assemble           Only assemble assembly code into AEXE. Will not start emulator.
```
```
  -r, --run                Run an already assembled program in AstroEXE format (program.AEXE)
```
```
  -nk, --nokeyboard        Use the mouse mode for the emulator (disables keyboard input)
```
```
  -v, --verbose            Write extra data to console for better debugging
```
```
  -f, --freq <value>       Override the default CPU target frequency with your own. Default = 10, higher = faster. 
                           High frequencies may be too hard to reach for some cpus
```


## Logisim
Along with the emulator, you can look at the actual circuit design for the system and even run your programs in it. 
1. Compile/Assemble your program using `Astro8-Emulator`. It will save the machine code to a file called `program_machine_code` automatically. This file is located directly next to the `Astro8-Emulator` executable.
2. Open the file called `cpu-circuit.circ` in the newest version of [Logisim Evolution](https://github.com/logisim-evolution/logisim-evolution/releases)
3. Locate the RAM area, and find the one called `MEMORY`
4. Right-click on it, and click `Load Image...`
5. In the file view that just appeared, locate your compiled program file directly next to the `Astro8-Emulator` executable, and click `Open`
6. Press play, and it should run.
