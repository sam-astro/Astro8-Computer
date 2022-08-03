---
layout : default
parent : Architecture
title : Instruction Set
nav_order : 3
---

# Instruction Set

## List:
0. [NOP](#nop)
1. [AIN](#ain)
2. [BIN](#bin)
3. [CIN](#cin)
4. [LDIA](#ldia)
5. [LDIB](#ldib)
6. [RDEXP](#rdexp)
7. [WREXP](#wrexp)
8. [STA](#sta)
9. [STC](#stc)
10. [ADD](#add)
11. [SUB](#sub)
12. [MULT](#mult)
13. [DIV](#div)
14. [JMP](#jmp)
15. [JMPZ](#jmpz)
16. [JMPC](#jmpc)
17. [JREG](#jreg)
18. [LDAIN](#ldain)
19. [STAOUT](#staout)
20. [LDLGE](#ldlge)
21. [STLGE](#stlge)
22. [LDW](#ldw)
23. [SWP](#swp)
24. [SWPC](#swpc)

<br>

## NOP
*No operation: acts as a fetch without an instruction.*

ID: `0`, `0b00000`

### Syntax:

```
NOP
```

<br>

## AIN

ID: `1`, `0b00001`

*Load data from the given address into register* ***A***

### Syntax

```
AIN 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## BIN

ID: `2`, `0b00010`

*Load data from the given address into register* ***B***

### Syntax

```
BIN 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## CIN

ID: `3`, `0b00011`

*Load data from the given address into register* ***C***

### Syntax

```
CIN 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## LDIA

ID: `4`, `00100`

*Load immediate value into register* ***A***

### Syntax

```
LDIA 〈 Ｖａｌｕｅ 〉
```

<br>

## LDIB

ID: `5`, `00101`

*Load immediate value into register* ***B***

### Syntax

```
LDIB 〈 Ｖａｌｕｅ 〉
```

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

