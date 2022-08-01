
# Micro Instructions

Logisim micro-instructions are in reversed order. <br>
Bottom of this list is left of binary code.

<br>

```
// 000000000000 11  compact
SU : enable subtraction in ALU
MU : enable multiplication in ALU
DI : enable division in ALU
```

<br>

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

```
// 11111 000000000  full
FL : update flags register
EI : end instruction, resets step counter to move to next instruction
ST : stop the clock
CE : enable incrementing of counter
EO : read from ALU to bus
```

<br>
