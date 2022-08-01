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
<br>

## Program Memory

Percent: `0 - 25%` <br>
Range: `0 - 16382` <br>
Words: `16382`

Contains the program instructions.

<br>
<br>

## Character Memory

Percent: `25-25%` <br>
Range: `16383 - 16527` <br>
Words: `144`

Contains the indexes of the characters to <br>
be displayed at the corresponding location.

<br>
<br>

## Variable Memory

Percent: `25-25%` <br>
Range: `16528 - 16656` <br>
Words: `128`

<br>
<br>

## Video Memory

Percent: `93-100%` <br>
Range: `61439 - 65535` <br>
Words: `4096`

<br>
