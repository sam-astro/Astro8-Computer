# Astro-8 Computer

This is a 16-bit computer design called the Astro-8. It has a fully functional design in the logic simulator called Logisim Evolution. I also made an emulator for running programs at full speed. Continue reading for installation instructions and how to use it.

## Installation:
### Windows
1. Navigate to [the most recent release](https://github.com/sam-astro/Astro8-Computer/releases), and download the **Windows** version
2. Unzip the downloaded file
### Linux
1. Navigate to [the most recent release](https://github.com/sam-astro/Astro8-Computer/releases), and download the **Linux** version
2. Unzip the downloaded file
### From Source
1. Clone this repository in a command line using `git clone https://github.com/sam-astro/Astro8-Computer.git` OR by downloading the repository as a .ZIP file and unzipping it to your location of choice
2. Enter the directory `Astro8-Computer/Astro8-Emulator/linux-build`
3. Run CMake using `cmake ..` to generate Unix Makefile
4. Run `make -j5` to generate executable
5. The executable is `Astro8-Computer/Astro8-Emulator/linux-build/Astro8-Emulator`

## Use
### Emulator
The file called `Astro8-Emulator` serves multiple purposes.
* Emulates machine code just like the real hardware
* Compiles Armstrong into assembly
* Assembles assembly into machine code and stores it into a file called `program_machine_code`

To run your code, you may either simply start the program where you will be prompted to input your code - OR, you can provide a path as a command line argument. You can either type directly into the command line (don't use any blank lines), or enter a path to your armstrong or assembly file and press enter ***twice***. The type will be determined by the first line of the file. ***All Armstrong files should have `#AS` as the first line.***

There is a second executable written in C# called `ResourceGenerator`. This is used to generate binary data from the character-set PNG file. Unless you want to change the font or add new characters, you don't need to use this.

### Logisim
Along with the emulator, you can look at the actual circuit design for the system and even run your programs in it. 
1. Compile/Assemble your program using `Astro8-Emulator`. It will save the machine code to a file called `program_machine_code` automatically. This file is located directly next to the `Astro8-Emulator` executable.
2. Open the file called `cpu-circuit.circ` in the newest version of [Logisim Evolution](https://github.com/logisim-evolution/logisim-evolution/releases)
3. Locate the RAM area, and find the one called `MEMORY`
4. Right-click on it, and click `Load Image...`
5. In the file view that just appeared, locate your compiled program file directly next to the `Astro8-Emulator` executable, and click `Open`
6. Press play, and it should run.

## Technical details:

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

### Microinstructions:
```
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

### Memory Layout:
```
word 0 |                                                       .                                                  | word 65535
       | Program mem. 0 - 16382    I                           I                          Video memory 61439-65535|

0-25%    (0 - 16382)   16382 words  -  Program mem.
25-25%   (16383-16527) 144 words    -  Character mem. (contains index of character to be displayed at the corresponding location)
25-25%   (16528-16656) 128 words    -  Variable mem.

93-100%  (61439-65535) 4096 words   -  Video mem. 
```



### Armstrong:
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

