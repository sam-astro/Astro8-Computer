---
layout : default
parent : Architecture
title : Instruction Set
nav_order : 3
---

# Instruction Set

## Unindexed:
* [SET](#set)
* [HERE](#here)

## Indexed:
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

## SET
*Set the specified memory location to a value during assembly*

### Syntax:

```
SET〈 Ａｄｄｒｅｓｓ 〉〈 Ｖａｌｕｅ 〉
```

<br>

## HERE
*Set the location of the `HERE` in memory equal to a value during assembly*

### Syntax:

```
HERE〈 Ｖａｌｕｅ 〉
```

<br>

## NOP
ID: `0`, `0b00000`

*No operation: acts as a fetch without an instruction.*

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

ID: `4`, `0b00100`

*Load immediate value into register* ***A***

### Syntax

```
LDIA 〈 Ｖａｌｕｅ 〉
```

<br>

## LDIB

ID: `5`, `0b00101`

*Load immediate value into register* ***B***

### Syntax

```
LDIB 〈 Ｖａｌｕｅ 〉
```

<br>

## RDEXP

ID: `6`, `0b00110`

Read from the expansion port into register A.

### Syntax

```
RDEXP
```

<br>

## WREXP

ID: `7`, `0b00111`

Write from register A onto the expansion port.

### Syntax

```
WREXP
```

<br>

## STA

ID: `8`, `0b01000`

Store register A into the given memory address.

### Syntax

```
STA 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## STC

ID: `9`, `0b01001`

Store register C into the given memory address.

### Syntax

```
STC 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## Math

10, `0b01010`   11, `0b01011`   12, `0b01100`   13, `0b01101`

Execute the mathematic operation on <br>
register A & B and save the result in A.

<br>

### Syntax
#### ADD
**A  +  B  🠖  A**

```
ADD
```

#### SUB
**A    -    B  🠖  A**

```
SUB
```

#### MULT
**A  ×  B  🠖  A**

```
MULT
```

#### DIV
**A**  **÷**  **B**  **🠖**  **A**

```
DIV
```

<br>

## Jump

14, `0b01110`   15, `0b01111`   16, `0b10000`   17, `0b10001`

Jumps to the given instruction position, <br>
done by setting the program counter. <br>
The address is given by the value stored <br>
directly after this instruction in memory


### Syntax
#### JMP
```
JMP
HERE〈 Ａｄｄｒｅｓｓ 〉
```

#### JMPZ
Jump if register **A** is zero
```
JMPZ
HERE〈 Ａｄｄｒｅｓｓ 〉
```

#### JMPC
Jump if the carry bit is set
```
JMPC
HERE〈 Ａｄｄｒｅｓｓ 〉
```

#### JREG
Jump to the value stored in register A
```
JREG
```

<br>

## LDAIN

ID: 18, `0b10010`

Use register **A**s value as memory address <br>
to load a value from RAM into register **A**

### Syntax
**Memory  🠖  A**
```
LDAIN
```

<br>

## STAOUT

ID: 19, `0b10011`

Use register **A**s value as memory address to then <br>
store the value inside of register **B** into RAM

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

## LDLGE

ID: 20, `0b10100`

Use value directly after instruction as <br>
address to read from memory into register **A**

### Syntax

```
LDLGE
HERE 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## STLGE

ID: 21, `0b10101`

Use value directly after counter as <br>
address to write from register **A** into memory

### Syntax

```
STLGE
HERE 〈 Ａｄｄｒｅｓｓ 〉
```

<br>

## LDW

ID: 22, `0b10110`

Load value directly after this into <br>
register **A**

### Syntax

```
LDW
HERE 〈 Ｖａｌｕｅ 〉
```

<br>

## Swap

23, `10111`   24, `11000`

Swaps two registers with each other <br>
& as a side-effect overwrites the third.

### Syntax
#### SWP
**A  ⟷  B   Overwrites C**
```
SWP
```

#### SWPC
**A  ⟷  C   Overwrites B** 
```
SWPC
```

<br>


