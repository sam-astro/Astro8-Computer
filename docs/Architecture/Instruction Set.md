---
layout : default
parent : Architecture
title : Instruction Set
nav_order : 3
---

# Instruction Set

## Instructions:
1. List of instructions
{:toc}

0. NOP
1. AIN
2. BIN
3. CIN
4. LDIA
5. LDIB
6. RDEXP
7. WREXP
8. STA
9. STC
10. ADD
11. SUB
12. MULT
13. DIV
14. JMP
15. JMPZ
16. JMPC
17. JREG
18. LDAIN
19. STAOUT
20. LDLGE
21. STLGE
22. LDW
23. SWP
24. SWPC

<br>

## No Operation

`00000`

<br>

### Syntax

```asm
NOP
```

<br>
<br>

## Load Data

`00001`   `00010`   `00011`

Load data from the given address into register  **A  -  C** .

<br>

### Syntax

```
AIN 〈 Ａｄｄｒｅｓｓ 〉
```

```
BIN 〈 Ａｄｄｒｅｓｓ 〉
```

```
CIN 〈 Ａｄｄｒｅｓｓ 〉
```

<br>
<br>

## Load Immediate

`00100`   `00101`

Load immediate value into register  **A  -  B** .

<br>

### Syntax

```
LDIA 〈 Ｖａｌｕｅ 〉
```

```
LDIB 〈 Ｖａｌｕｅ 〉
```

<br>
<br>

## Expansion Port

`00110`   `00111`

Read from / write to the expansion port to / from register B.

<br>

### Syntax

**Expansion Port 🠖 B**

```
RDEXP
```

**Expansion Port 🠔 B**

```
WREXP
```

<br>
<br>

## Store Value

`01000`   `01001`

Store the register value at the given memory address.

<br>

### Syntax

**A 🠖 Memory**

```
STA 〈 Ａｄｄｒｅｓｓ 〉
```

**C 🠖 Memory**

```
STC 〈 Ａｄｄｒｅｓｓ 〉
```

<br>
<br>

## Math

`01010`   `01011`   `01100`   `01101`

Execute the mathematic operation on <br>
register A & B and save the result in A.

<br>

### Syntax

**A  +  B  🠖  A**

```
ADD
```

**A    -    B  🠖  A**

```
SUB
```

**A  ×  B  🠖  A**

```
MULT
```

**A**  **÷**  **B**  **🠖**  **A**

```
DIV
```

<br>
<br>

## Jump

`01110`   `01111`   `10000`

Jumps to the given instruction position, <br>
which intern sets the program counter.

<br>

### Syntax

```
JMP 〈 Ｖａｌｕｅ 〉
```

Jump if register **A** is zero

```
JMPZ 〈 Ｖａｌｕｅ 〉
```

Jump if the carry bit is set

```
JMPC 〈 Ｖａｌｕｅ 〉
```

<br>
<br>

## Exchange Memory

`10001`   `10010`

Use register **A**s value as memory address <br>
to either load or store a memory value.

<br>

### Syntax

**Memory  🠖  A**

```
LDAIN
```

**B  🠖  Memory**

```
STAOUT
```

<br>
<br>

## LDLGE

`10011`

Use value directly after instruction <br>
as address to copy from memory <br>
to reg A and advance counter by 2.

<br>

### Syntax

```
LDLGE
```

<br>
<br>

## STLGE

`10100`

Use value directly after counter as <br>
address, then copy value from reg A <br>
to memory and advance counter by 2.

<br>

### Syntax

```
STLGE
```

<br>
<br>

## Swap

`10101`   `10110`

Swaps two registers with each other <br>
& as a side-effect overrides the third.

<br>

### Syntax

**A  ⟷  B   Overrides C**

```
SWP
```

**A  ⟷  C   Overrides B** 

```
SWPC
```

<br>
<br>

## Halt

`10111`

Stop the clock and thus execution.

<br>

### Syntax

```
HLT
```

<br>

