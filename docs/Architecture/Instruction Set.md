---
layout : default
parent : Architecture
title : Instruction Set
category: Architecture
nav_order : 3
---

# Instruction Set

{: .tip .caps}
The Instruction Set is also called by it's name, `Astrisc`, and also sometimes called Assembly

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
<br>    ~~[RDEXP](#rdexp)~~ *(Removed in v2.0.0-alpha)*
<br>    ~~[WREXP](#wrexp)~~ *(Removed in v2.0.0-alpha)*
6. [STA](#sta)
<br>    ~~[STC](#stc)~~ *(Removed in v2.0.0-alpha)*
7. [ADD](#add)
8. [SUB](#sub)
9. [MULT](#mult)
10. [DIV](#div)
11. [JMP](#jmp)
12. [JMPZ](#jmpz)
13. [JMPC](#jmpc)
14. [JREG](#jreg)
15. [LDAIN](#ldain)
16. [STAOUT](#staout)
17. [LDLGE](#ldlge)
18. [STLGE](#stlge)
19. [LDW](#ldw)
20. [SWP](#swp)
21. [SWPC](#swpc)
22. [PCR](#pcr)
23. [BSL](#bsl)
24. [BSR](#bsr)
25. [AND](#and)
26. [OR](#or)
27. [NOT](#not)
28. [BNK](#bnk)
29. [VBUF](#vbuf)
30. [BNKC](#bnkc)
31. [LDWB](#ldwb)

<br>

## SET
*Set the specified memory location to a value during assembly*

#### Syntax:

```
SET <Address> <Value>
```

<br>

## HERE
*Set the location of the `HERE` in memory equal to a value during assembly*

#### Syntax:

```
HERE <Value>
```

<br>

## NOP
ID: `0`

*No operation: acts as a fetch without an instruction.*

#### Syntax:

```
NOP
```

<br>

## AIN

ID: `1`

*Load data from the given address into register* ***A***

#### Syntax

```
AIN <Address>
```

<br>

## BIN

ID: `2`

*Load data from the given address into register* ***B***

#### Syntax

```
BIN <Address>
```

<br>

## CIN

ID: `3`

*Load data from the given address into register* ***C***

#### Syntax

```
CIN <Address>
```

<br>

## LDIA

ID: `4`

*Load immediate value into register* ***A***

#### Syntax

```
LDIA <Value>
```

<br>

## LDIB

ID: `5`

*Load immediate value into register* ***B***

#### Syntax

```
LDIB <Value>
```

<br>

## RDEXP
*(Removed in v2.0.0-alpha)*

~~ID: `6`, `0b00110`~~

Read from the expansion port into register A.

#### Syntax

```
RDEXP
```

<br>

## WREXP
*(Removed in v2.0.0-alpha)*

~~ID: `7`, `0b00111`~~

Write from register A onto the expansion port.

#### Syntax

```
WREXP
```

<br>

## STA

ID: `6`

Store register A into the given memory address.

#### Syntax

```
STA <Address>
```

<br>

## STC
*(Removed in v2.0.0-alpha)*

~~ID: `9`, `0b01001`~~

Store register C into the given memory address.

#### Syntax

```
STC <Address>
```

<br>

## Math

IDs: 7    8    9    10

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

IDs: 11    12    13    14

Jumps to the given instruction position, <br>
done by setting the program counter. <br>
The address is given by the value stored <br>
directly after this instruction in memory


### Syntax
#### JMP
```
JMP
HERE <Address>
```

#### JMPZ
Jump if register **A** is zero
```
JMPZ
HERE <Address>
```

#### JMPC
Jump if the carry bit is set
```
JMPC
HERE <Address>
```

#### JREG
Jump to the value stored in register A
```
JREG
```

<br>

## LDAIN

ID: 15

Use register **A**s value as memory address <br>
to load a value from RAM into register **A**

#### Syntax
**Memory  🠖  A**
```
LDAIN
```

<br>

## STAOUT

ID: 16

Use register **A**s value as memory address to then <br>
store the value inside of register **B** into RAM

<br>

#### Syntax
**B  🠖  Memory**
```
STAOUT
```

<br>

## LDLGE

ID: 17

Use value directly after instruction as <br>
address to read from memory into register **A**

#### Syntax

```
LDLGE
HERE <Address>
```

<br>

## STLGE

ID: 18

Use value directly after counter as <br>
address to write from register **A** into memory

#### Syntax

```
STLGE
HERE <Address>
```

<br>

## LDW

ID: 19

Load value directly after this into <br>
register **A**

#### Syntax

```
LDW
HERE <Value>
```

<br>

## Swap

IDs: 20    21

Swaps two registers with each other <br>
& as a side-effect overwrites the third.

#### Syntax
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


## PCR

ID: 22

Get the current program counter value <br>
and put it into register **A**

#### Syntax

```
PCR
```

<br>

## BSL

ID: 23

Bit shift left register **A**, the number<br>
of bits to shift determined by the value<br>
in register B

#### Syntax

```
LDIA 10
LDIB 2
BSL     , Shift value of 10 to the left by 2
```

<br>

## BSR

ID: 24

Bit shift right register **A**, the number<br>
of bits to shift determined by the value<br>
in register B

#### Syntax

```
LDIA 10
LDIB 2
BSR     , Shift value of 10 to the right by 2
```

<br>

## AND

ID: 25

Bitwise And operation on register **A**<br>
and register **B**

#### Syntax

```
LDIA 15
LDIB 1
AND     , Bitwise And operation on 0b1111 and 0b0001
```

<br>

## OR

ID: 26

Bitwise Or operation on register **A**<br>
and register **B**

#### Syntax

```
LDIA 15
LDIB 1
OR     , Bitwise Or operation on 0b1111 and 0b0001
```

<br>

## NOT

ID: 27

Bitwise Not operation on register **A**

#### Syntax

```
LDIA 15
NOT     , Bitwise Not operation on 0b1111
```

<br>

## BNK

ID: 28

Change the memory bank register to the value specified

#### Syntax

```
BNK 1
```

<br>

## VBUF

ID: 29

Copy the video buffer to video card - displays the editable video memory

#### Syntax

```
VBUF
```

<br>

## BNKC

ID: 30

Change the memory bank register to the value<br>
in register **C**

#### Syntax

```
LDIA 15  , Load 15 into A then swap it into C
SWPC
BNKC     , Sets the memory bank register to C, which is 15
```

<br>

## LDWB

ID: 31

Load value directly after counter into <br>
register **B** and advance program counter by 2

#### Syntax

```
LDWB
HERE <Value>
```

<br>

