### Instruction set:

```
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
LDLGE       10010   : use value directly after instruction as address to copy from memory to reg A and advance counter by 2
STLGE       10100   : use value directly after counter as address, then copy value from reg A to memory and advance counter by 2
SWP         10011   : swap the contents of register A and register B (this overwrites register C)
SWPC        10110   : swap register A and register C (this overwrites register B)
HLT         10111   : stop the clock
```