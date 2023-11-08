---
layout : default
parent : Architecture
title : Micro Instructions
category: Architecture
nav_order : 4
indent: 1
---

# Micro Instructions

Microinstructions are the hardware-level of what instructions actually are. They are singular values, which can be either true or false. In hardware, this would simply be a wire turning something on or off.

This is necessary in order to break-down instructions into their base parts, because they cannot be executed all at once, since instructions often require multiple steps, even using the same part of the CPU.

{: .note .caps}
Many of the microinstructions are placed in groups. This is done to reduce the amount of index space they require in binary form. For example, there are 7 different [Read Microinstructions](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Micro%20Instructions.html#read-microinstructions). But since all of them write a value to the bus, we can obviously only use one at a time, unless we want to mess up our data. So instead of having a single bit dedicated to every single one to tell our CPU which to turn on or off, we can just group them together and index them. <br/> In the sample below, this means what would be `1111111` can be reduced to `111`, saving 4 bits for other instructions. 

<br>

### ALU Microinstructions:

```
// 000000000000 11  compact
SU : enable subtraction in ALU
MU : enable multiplication in ALU
DI : enable division in ALU
```

<br>

### Read Microinstructions:

```
// 000000000 111 00  compact
RA : read from reg A to bus
RB : read from reg B to bus
RC : read from reg C to bus
RM : read from memory to bus at the address in mem addr. register
IR : read from lowest 12 bits of instruction register to bus
CR : read value from counter to bus
RE : read from expansion port to bus
```

<br>

### Write Microinstructions:

```
// 00000 1111 00000  compact
WA : write from bus to reg A
WB : write from bus to reg B
WC : write from bus to reg C
IW : write from bus to instruction register
DW : write from bus to display register
WM : write from bus to memory
J  : write from bus to counter current value
AW : write lowest 12 bits from bus to mem. addr. register
WE : write from bus to expansion port
```

<br>

### Uncategorized Microinstructions:

```
// 11111 000000000  full
FL : update flags register
EI : end instruction, resets step counter to move to next instruction
ST : stop the clock
CE : enable incrementing of counter
EO : read from ALU to bus
```

<br>
