
# Instruction Set

<br>

## No Operation

`00000`

### Syntax

<kbd>  NOP  </kbd>

<br>
<br>

## Load Data

`00001`   `00010`   `00011`

Load data from the given address into register  **A  -  C** .


### Syntax

<kbd>  AIN  </kbd>   <kbd>  \< Address \>  </kbd> <br>
<kbd>  BIN  </kbd>   <kbd>  \< Address \>  </kbd> <br>
<kbd>  CIN  </kbd>   <kbd>  \< Address \>  </kbd>

<br>
<br>

## Load Immediate

`00100`   `00101`

Load immediate value into register  **A  -  B** .


### Syntax

<kbd>  LDIA  </kbd>   <kbd>  \< Value \>  </kbd> <br>
<kbd>  LDIB  </kbd>   <kbd>  \< Value \>  </kbd>

<br>
<br>

## Expansion Port

`00110`   `00111`

Read from / write to the expansion port to / from register B.

### Syntax

<kbd>  RDEXP  </kbd>   **Expansion Port**  **🠖**  **B** <br>
<kbd>  WREXP  </kbd>   **Expansion Port**  **🠔**  **B**

<br>
<br>

## Store Value

`01000`   `01001`

Store the register value at the given memory address.

### Syntax

<kbd>  STA  </kbd>   <kbd>  \< Address \>  </kbd>   **A**  **🠖**  **Memory** <br>
<kbd>  STC  </kbd>   <kbd>  \< Address \>  </kbd>   **C**  **🠖**  **Memory**

<br>
<br>

## Math

`01010`   `01011`   `01100`   `01101`

Execute the mathematic operation on <br>
register A & B and save the result in A.

### Syntax

<kbd>       ADD       </kbd>   **A**  **+**  **B**  **🠖**  **A** <br>
<kbd>       SUB       </kbd>   **A**   **-**    **B**  **🠖**  **A** <br>
<kbd>  MULT  </kbd>   **A**  **×**  **B**  **🠖**  **A** <br>
<kbd>       DIV       </kbd>   **A**  **÷**  **B**  **🠖**  **A** 

<br>
<br>

## Jump

`01110`   `01111`   `10000`

Jumps to the given instruction position, <br>
which intern sets the program counter.

### Syntax

<kbd>       JMP       </kbd>   <kbd>  \< Value \>  </kbd> <br>
<kbd>  JMPZ  </kbd>   <kbd>  \< Value \>  </kbd>   Jump if register **A** is zero <br>
<kbd>  JMPC  </kbd>   <kbd>  \< Value \>  </kbd>   Jump if the carry bit is set

<br>
<br>

## Exchange Memory

`10001`   `10010`

Use register **A**s value as memory address <br>
to either load or store a memory value.

### Syntax

<kbd>       LDAIN      </kbd>   **Memory**  **🠖**  **A** <br>
<kbd>  STAOUT  </kbd>   **B**  **🠖**  **Memory** <br>

<br>
<br>

## LDLGE

`10011`

Use value directly after instruction <br>
as address to copy from memory <br>
to reg A and advance counter by 2.

### Syntax

<kbd>       LDLGE      </kbd>

<br>
<br>

## STLGE

`10100`

Use value directly after counter as <br>
address, then copy value from reg A <br>
to memory and advance counter by 2.

### Syntax

<kbd>     STLGE      </kbd>

<br>
<br>

## Swap

`10101`   `10110`

Swaps two registers with each other <br>
& as a side-effect overrides the third.

### Syntax

<kbd>       SWP       </kbd>   **A**  **⟷**  **B**   Overrides **C** <br>
<kbd>  SWPC  </kbd>   **A**  **⟷**  **C**   Overrides **B** 


<br>
<br>

## Halt

`10111`

Stop the clock and thus execution.

### Syntax

<kbd>  HLT  </kbd>

<br>

