---
layout : default
parent : Architecture
title : Memory Layout
nav_order : 1
---

# Memory Layout

<br>

```
word 0  |                                         .                                     | word 65535
        | Program mem. 0 - 16382    I             I             Video memory 61439-65535|
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
