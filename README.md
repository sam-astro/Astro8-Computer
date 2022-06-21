# Astro-8 Computer

```
~  SET <addr> <val>   : tells assembler to set <addr> equal to <val>
0  NOP         0000   : no operation
1  LODA <addr> 0001   : load data from <addr> to reg A
2  LODB <addr> 0010   : load data from <addr> to reg B
3  ADD <addr>  0011   : add value at <addr> to reg A, and set reg A = to sum
4  SUB <addr>  0100   : subtract value at <addr> from reg A, and set reg A = to sum
5  OUT         0101   : copy value from reg A to display reg
6  JMP <val>   0110   : change counter to <val> (changes which instruction is next)
7  STA <addr>  0111   : store value of A into <addr> of memory
8  LDI <val>   1000   : immediately load <val> into reg A
9  JMPZ <val>  1001   : jump to <val> if the value in reg A is equal to zero
10 JMPC <val>  1010   : jump if the carry bit is set
11 HLT         1011   : stop the clock
12 LDAIN       1100   : load from reg A as memory address, then copy value from memory into A (allows for 16-bit addressing)
13 
14 
15 
16

microinstructions
0  SU : enable subtraction in ALU
1  IW : write from bus to instruction register
2  DW : write from bus to display register
3  ST : stop the clock
4  CE : enable incrementing of counter
5  CR : read value from counter to bus
6  WM : write from bus to memory
7  RA : read from reg A to bus
8  EO : read from ALU to bus
9  FL : write values to flags register
10 J  : write from bus to counter current value
11 WB : write from bus to reg B
12 WA : write from bus to reg A
13 RM : read from memory to bus at the address in mem addr. register
14 AW : write lowest 12 bits from bus to mem. addr. register
15 IR : read from lowest 12 bits of instruction register to bus
16 EI : end instruction, resets step counter to move to next instruction

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

@                            - '@' symbol refers to a register, ie. ('@A' for register 'A')
#                            - '#' symbol refers to a label, which is a point the program can jump to ie. ('#main')
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
