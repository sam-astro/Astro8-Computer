# Astro-8 Computer

```
instructions

NOP         00000   : no operation
AIN <addr>  00001   : load data from <addr> to reg A
BIN <addr>  00010   : load data from <addr> to reg B
CIN <addr>  00011   : load data from <addr> to reg C
LDIA <val>  00100   : immediately load <val> into reg A
LDIB <val>  00101   : immediately load <val> into reg B
RDEXP       00110   : load value stored on the expansion port into reg B
WREXP       00111   : copy reg A into the expansion port
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
STAOUT      10010   : use reg A as memory address, then copy value from B into memory
LDLGE       10011   : use value directly after this instruction as the address, then copy from memory at that address to reg A and advance counter by 2
STLGE       10100   : use value directly after counter as address, then copy value from reg A to memory and advance counter by 2
SWP         10101   : swap the contents of register A and register B (this will overwrite the contents of register C, using it as a temporary swap area)
SWPC        10110   : swap register A and register C (this will overwrite the contents of register B, using it as a temporary swap area)
HLT         10111   : stop the clock



microinstructions

(logisim microinstructions are in reversed order. Bottom of this list is left of binary code )

// 000000000000 11  compact
SU : enable subtraction in ALU
MU : enable multiplication in ALU
DI : enable division in ALU


// 000000000 111 00  compact
RA : read from reg A to bus
RB : read from reg B to bus
RC : read from reg C to bus
RM : read from memory to bus at the address in mem addr. register
IR : read from lowest 12 bits of instruction register to bus
CR : read value from counter to bus
RE : read from expansion port to bus


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

// 11111 000000000  full
FL : update flags register
EI : end instruction, resets step counter to move to next instruction
ST : stop the clock
CE : enable incrementing of counter
EO : read from ALU to bus
```

character rom = 1040 bytes

Memory Layout:
```
word 0 |                                                       .                                                  | word 65535
       | Program mem. 0 - 16382    I                           I                          Video memory 61439-65535|

0-25%    (0 - 16382)   16382 words  -  Program mem.
25-25%   (16383-16527) 144 words    -  Character mem. (contains index of character to be displayed at the corresponding location)
25-25%   (16528-16656) 128 words    -  Variable mem.

93-100%  (61439-65535) 4096 words   -  Video mem. 
```


Multiply program:
```
, Set the first factor
ldia 4
, Set the second factor
ldib 5
, Multiply
mult
out
hlt
```

New Assembly (WIP):
```
// Use hex values (0xff) when referring to addresses, and decimal (213) for a new immediate integer

@A - A register
@B - B register
@C - C register
@EX - Expansion port

$                              - '$' symbol refers to a variable containing an integer (address in memory)
@                              - '@' symbol refers to a register, ie. ('@A' for register 'A')
#                              - '#' symbol refers to a label, which is a point the program can jump to ie. ('#main') (address in program counter)
define <addr> <val>            - Assembler defines <addr> equal to <val>. Sets memory values before the program is executed, then is removed.
change <addr> = <val>          - Change <addr> to <val> at any time
add <valA>,<valB> -> <addr>    - Add the values <valA> and <valB>, then store the result in <addr>
sub <valA>,<valB> -> <addr>    - Subtract the values <valA> and <valB> (valA - valB), then store the result in <addr>
mult <valA>,<valB> -> <addr>   - Multiply the values <valA> and <valB>, then store the result in <addr>
div <valA>,<valB> -> <addr>    - Divide the values <valA> and <valB> (valA / valB), then store the result in <addr>
goto <addr>                    - Jumps to the given address or label
gotoif <valA><C><valB>,<addr>  - Jumps to <addr>, given the logic relationship between <valA> and <valB> given a comparer <C>, ie. (jmpc 0x12==4,0x0)
if <valA><C><valB>             - Continues if the logic relationship between <valA> and <valB> given a comparer <C> is true, ie. (if 0x12==4)
endif                          - Marks the ending of the contents of an if statement
```

High-level:
```
set 0x1ff 32    // Create 32 for multiplying G
set 0x1fe 1024  // Create 1024 for multiplying R
set 0x3e8 1     // Constant 1

div 0x12a,2 -> 0x120  // Divide x-location by 2
div 0x12b,2 -> 0x121  // Divide y-location by 2

//   Red is equal to x * 10-bit offset of 1024
mult 0x120,0x1fe -> 0x12c

//   Green is equal to y * 5-bit offset of 32
mult 0x121,0x1ff -> 0x12d

//   Blue is equal to 63 - ( ( x + y ) / 2)
add 0x120,0x121 -> @A
div @A,4 -> @B
sub 63,@B -> 0x12e

// Add RGB values and output value
add 0x12c,0x12d -> @A
add @A,0x12e -> @A
out @A


// Handle incrementing x and y, and resetting when above 64 (screen size)
add 0x12a,1 -> 0x12a
jmpc 0x12a==64,#incrementY  // If X is equal to 64, jump to #incrementY
jmp 0x0 // Else return to top

#incrementY
add 0x12b,1 -> 0x12b  // Increment Y by 1, and reset X to 0
change 0x12a = 0
jmpc 0x12b==64,#resetY // If Y is equal to 64, jump to #resetY
jmp 0x0 // Else return to top

#resetY
change 0x12b = 0 // Reset Y to 0
jmp 0x0 // Finally return to top
```

Parsed:
```
set 511 32    // Create 32 for multiplying G
set 510 1024  // Create 1024 for multiplying R
set 1000 1     // Constant 1

// Divide x-location by 2
ain 298
ldib 2
div
sta 288
// Divide y-location by 2
ain 299
ldib 2
div
sta 289

//   Red is equal to x * 10-bit offset of 1024
ain 288
bin 510
mult
sta 300

//   Green is equal to y * 5-bit offset of 32
ain 289
bin 511
mult
sta 301

//   Blue is equal to 63 - ( ( x + y ) / 2)
ain 288
bin 289
add
ldib 4
div
swp
ldia 63
sub
sta 302

// Add RGB values and output value
ain 300
bin 301
add
bin 302
add
out


// Handle incrementing x and y, and resetting when above 64 (screen size)
ain 298
ldib 1
add
sta 298
swp
ldia 64
sub
jmpz 40
jmp 0

ain 299
ldib 1
add
sta 299
ldia 0
sta 298
bin 299
ldia 64
sub
jmpz 51
jmp 0

ldia 0
sta 299
jmp 0
```
