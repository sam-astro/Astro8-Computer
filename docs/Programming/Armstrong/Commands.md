---
layout : default
parent: Armstrong
grand_parent: Programming
title : Commands
---

# Commands

*Available **Armstrong** functions.*

<br>

## Define

Macro that sets a memory address to the <br>
given values before the program is executed. <br>
Only constants can be used as Values.

### Syntax

```
define <Address> <Value>
```

### Example

```
define 0xff2 33
define $var 6
```

<br>
<br>

## Change

Assigns the given address the supplied value.

### Syntax

```
change <Address> = <Value>
```

### Example

```
change 0xff2 = 19
change $cursorChar = 8
```

<br>
<br>

## Add / Subtract / Multiply / Divide

Combines the two given values depending on the <br>
operator and saves the result at the given address.<br>
The outout location is prefixed by an arrow, ` -> ` 

### Syntax

```
<Operator> <Value A>, <Value B> -> <Output location>
```

### Example

```
add $X , $Y -> $Z  //  Z = X + Y
mult $X , $Y -> $Z //  Z = X * Y
sub $X , $Y -> $Z  //  Z = X - Y
div $X , $Y -> $Z  //  Z = X / Y
and $X , $Y -> $Z  //  Z = X & Y
or $X , $Y -> $Z   //  Z = X | Y
not $X -> $Z       //  Z = ~ X
bsl $X , $Y -> $Z  //  Z = X << Y
bsr $X , $Y -> $Z  //  Z = X >> Y
```

<br>
<br>

## Goto

Jumps to the given address or label.

### Syntax

```
goto <Address>
```

### Example

```
goto #mainLoop
```

<br>
<br>

## Goto If

Jumps to the given address or label if the <br>
comparison of the two values return true.

### Syntax

```
gotoif <Value A> <Comparator> <Value B> , <Address>
```

### Example

```
gotoif $var < 4095 , #colorScreen
gotoif $var == 4095 , #colorScreen
```

<br>
<br>

## If

Enters the logic block if the comparison <br>
of the two given values returns true.

### Syntax

```
if <Value A> <Comparator> <Value B> :

    <Contents>
    
endif
```

### Example

```
if $ballPosY > 62:
    change $ballVelY = 1
endif
```

<br>
