---
layout : default
parent : Architecture
title : Memory Layout
nav_order : 1
---

> The memory layout was changed as of version **v1.0.0-alpha**. If you have an earlier version less than **v1.0.0-alpha**, [click here](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Memory%20Layout.html#memory-layout-v032-alpha--) to view the old memory layout.

# Memory Layout v1.0.0-alpha +

<br>

```
        |                   |                                          |   |           |
word 0  | ░░░░░░░░░░░░░░░░░ | ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓| ▒ | ░░░░░░░░░ | word 65535
        | Program mem.             General mem.               Char mem.   Video memory |
```

<br>

## Program Memory

Percent: `0 - 25%` <br>
Range: `0 - 16381` <br>
Words: `16381`

Contains the program instructions.

<br>

## Variable and General Purpose Memory

Percent: `25-93%` <br>
Range: `16382 - 61293` <br>
Words: `44910`

Memory that the compiler automatically <br>
allocates as needed for variables, or you <br>
can use for other general purposes.

<br>

## Character Memory

Percent: `25-25%` <br>
Range: `61294 - 61438` <br>
Words: `145`

Contains the indexes of the characters to <br>
be displayed at the corresponding location.

<br>

## Video Memory

Percent: `93-100%` <br>
Range: `61439 - 65535` <br>
Words: `4096`

Contains the pixel's color data to be <br>
displayed at the corresponding location.

<br>
<br>


# Memory Layout v0.3.2-alpha -

<br>

```
        |                   |   |                                           |           |
word 0  | ░░░░░░░░░░░░░░░░░ | ▒ | ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ | ░░░░░░░░░ | word 65535
        | Program mem.     Char mem.        General mem.                   Video memory |
```

<br>

## Program Memory

Percent: `0 - 25%` <br>
Range: `0 - 16381` <br>
Words: `16381`

Contains the program instructions.

<br>

## Character Memory

Percent: `25-25%` <br>
Range: `16382 - 16527` <br>
Words: `145`

Contains the indexes of the characters to <br>
be displayed at the corresponding location.

<br>

## Variable and General Purpose Memory

Percent: `25-93%` <br>
Range: `16528 - 61438` <br>
Words: `44910`

Memory that the compiler automatically <br>
allocates as needed for variables, or you <br>
can use for other general purposes.

<br>

## Video Memory

Percent: `93-100%` <br>
Range: `61439 - 65535` <br>
Words: `4096`

Contains the pixel's color data to be <br>
displayed at the corresponding location.

<br>
