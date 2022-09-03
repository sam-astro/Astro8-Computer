---
has_children : true
nav_order : 4
layout : default
title : Programming
---

# Programming

*Getting started in **Armstrong**.*

<br>

[![Button Syntax]][Syntax]   
[![Button Examples]][Examples] 

<br>

## Before we start:

1.  Create a file named `main.armstrong`
2.  Every **Armstrong** file must start with `#AS`, so make that the very first line

<br>

## Hello world:

In Armstrong, the way to change characters and pixels on screen is by directly changing memory locations.
This is similar to the `POKE` command in BASIC.
You can use the `change` command in Armstrong like so:
```
change <location> = <value>
```
Now we want to change the location in memory that corresponds to character locations.
Character memory starts at `16382`, which is the top left of the screen, and moves from left to right for the rest of the locations.
Now to write the text "Hello World" we will do this:
```
// Hello
change 16382 = 'h'
change 16383 = 'e'
change 16384 = 'l'
change 16385 = 'l'
change 16386 = 'o'

// World
change 16392 = 'w'
change 16393 = 'o'
change 16394 = 'r'
change 16395 = 'l'
change 16396 = 'd'
```

<br>

## Syntax Highlighting

Visual Studio Code has a syntax extension for Armstrong which you can get here:

[![Button VSCode]][Extension VSCode]

<br>


<!----------------------------------------------------------------------------->

[Extension VSCode]: https://marketplace.visualstudio.com/items?itemName=sam-astro.armstrong
[Examples]: https://github.com/sam-astro/Astro8-Computer/tree/main/example_armstrong_programs

[Syntax]: Armstrong/README


<!---------------------------------[ Buttons ]--------------------------------->

[Button Examples]: https://img.shields.io/badge/Examples-00979D?style=flat-square&logoColor=white&logo=AppleArcade
[Button Syntax]: https://img.shields.io/badge/Syntax-CB2E6D?style=flat-square&logoColor=white&logo=AzureFunctions
[Button VSCode]: https://img.shields.io/badge/VSCode-007ACC?style=flat-square&logoColor=white&logo=VisualStudioCode
