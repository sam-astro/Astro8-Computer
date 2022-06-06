# Basic-CPU-In-Logisim

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
10  
11 
12 
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
