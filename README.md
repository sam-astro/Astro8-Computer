# Astro-8 Computer

```
instructions

NOP         00000   : no operation
AIN <addr>  00001   : load data from <addr> to reg A
BIN <addr>  00010   : load data from <addr> to reg B
CIN <addr>  00011   : load data from <addr> to reg C
LDIA <val>  00100   : immediately load <val> into reg A
LDIB <val>  00101   : immediately load <val> into reg B
LDEXP       00110   : load value stored on the expansion port into reg B
STEXP       00111   : copy reg A into the expansion port
STA <addr>  01000   : store value of A into <addr> of memory
STC <addr>  01001   : store value of C into <addr> of memory
ADD         01010   : add reg B to reg A, and set reg A = to sum
SUB         01011   : subtract reg B from reg A, and set reg A = to sum
MULT        01100   : multiply reg B with reg A, and set reg A = to product
DIV         01101   : divide reg A by reg B, and set reg A = to quotient
JMP <val>   01110   : change counter to <val> (changes which instruction is next)
JMPZ <val>  01111   : jump to <val> if the value in reg A is equal to zero
JMPC <val>  10000   : jump if the carry bit is set
LDAIN       10001   : load from reg A as memory address, then copy value from memory into A (allows for 16-bit addressing)
HLT         10010   : stop the clock
OUT         10011   : copy value from reg A to display reg



microinstructions

(logisim microinstructions are in reversed order. Bottom of this list is left of binary code )

// 0000000000000 1  full
EO : read from ALU to bus

// 00000000000 11 0  compact
SU : enable subtraction in ALU
MU : enable multiplication in ALU
DI : enable division in ALU


// 00000000 111 000  compact
RA : read from reg A to bus
RB : read from reg B to bus
RC : read from reg C to bus
RM : read from memory to bus at the address in mem addr. register
IR : read from lowest 12 bits of instruction register to bus
CR : read value from counter to bus
RE : read from expansion port to bus


// 0000 1111 000000  compact
WA : write from bus to reg A
WB : write from bus to reg B
WC : write from bus to reg C
IW : write from bus to instruction register
DW : write from bus to display register
WM : write from bus to memory
J  : write from bus to counter current value
AW : write lowest 12 bits from bus to mem. addr. register
WE : write from bus to expansion port

// 1111 0000000000  full
FL : update flags register
EI : end instruction, resets step counter to move to next instruction
ST : stop the clock
CE : enable incrementing of counter
```

Multiply program:
```
, Set the first factor
set 15 3
, Set the second factor
set 14 2
, Create value of 1
set 12 1
,
loda 15
sub 12
jmpc 6
loda 13
out
hlt
sta 15
loda 13
add 14
sta 13
jmp 0
```

New Assembly (WIP):
```
// Use hex values (0xff) when referring to addresses, and decimal (213) for a new immediate integer

@A - A register
@B - B register
@C - C register
@D - D register
@EX - Expansion port

$                            - '$' symbol refers to a variable containing an integer (address in memory)
@                            - '@' symbol refers to a register, ie. ('@A' for register 'A')
#                            - '#' symbol refers to a label, which is a point the program can jump to ie. ('#main') (address in program counter)
const <addr> <val>           - Assembler sets <addr> to <val>. Sets memory values before the program is executed, then is removed.
set <addr> <val>             - Change <addr> to <val> at any time
add <valA>,<valB> -> <addr>  - Add the values <valA> and <valB>, then store the result in <addr>
sub <valA>,<valB> -> <addr>  - Subtract the values <valA> and <valB> (valA - valB), then store the result in <addr>
mult <valA>,<valB> -> <addr> - Multiply the values <valA> and <valB>, then store the result in <addr>
div <valA>,<valB> -> <addr>  - Divide the values <valA> and <valB> (valA / valB), then store the result in <addr>
jmp <addr>                   - Jumps to the given address or label
jmpc <valA><C><valB>,<addr>  - Jumps to <addr>, given the logic relationship between <valA> and <valB> given a comparer <C>, ie. (jmpc 0x12==4,0x0)
```

```
,   Create 0 00000 00001 00000 32 for multiplying G
const 0x1ff 32
,   Create 0 00001 00000 00000 1024 for multiplying R
const 0x1fe 1024
,   Constant 1
const 0x3e8 1
,   0x120 = 0x12a / 2
div 0x12a,2 -> 0x120
,   0x121 = 0x12b / 2
div 0x12b,2 -> 0x121
,
,   Red is equal to the x times 10-bit offset of 1024
mult 0x120,0x1fe -> 0x12c
,
,   Green is equal to the y times 5-bit offset of 32
mult 0x121,0x1ff -> 0x12d
,
,   Blue is equal to 63 minus ( ( x plus y ) divided by 2)
add 0x120,0x121 -> @D
div @D,4 -> @D
sub 63,@D -> 0x12e
,
,    output  86 v
add 0x12c,0x12d -> 0x12c
add 0x12c,0x12e -> @D
out @D
,    incrementer
add 0x12a,1 -> 0x12a
jmpc 0x12a==64,#incrementY
jmp 0x0
,
#incrementY
add 0x12b,1 -> 0x12b
set 0x12a,0
jmpc 0x12b==64,#resetY
jmp 0x0
,
#resetY
set 0x12b,0
jmp 0x0
```
