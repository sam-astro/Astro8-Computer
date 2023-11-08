---
layout : default
parent: Armstrong
grand_parent: Programming
title : Armstrong Commands
category: Programming
nav_order: 3
indent: 1
---

# Commands

*Available **Armstrong** functions.*

<br>

## Define

Macro that sets a memory address to the <br>
given values before the program is executed. <br>
Only constants can be used as Values.

### Syntax

```haskell
define <Address> <Value>
```

As of v2.0.0-alpha you can also use an equals sign, `=` , between the Address and Value. This is *optional* and does not remove the previous functionality.

```haskell
define <Address> = <Value>
```

### Example

```haskell
define 0xff2 33
define $var = 6
```

<br>
<br>

## Change

Assigns the given address the supplied value.

### Syntax

```haskell
change <Address> = <Value>
```

### Example

```haskell
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

```php
<Operator> <Value A>, <Value B> -> <Output_location>
```

### Example

```php
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

```haskell
goto <Address>
```

### Example

```haskell
goto #mainLoop
```

<br>
<br>

## Goto If

Jumps to the given address or label if the <br>
comparison of the two values return true.

### Syntax

```haskell
gotoif <Value A> <Comparator> <Value B> , <Address>
```

### Example

```haskell
gotoif $var < 4095 , #colorScreen
gotoif $var == 4095 , #colorScreen
```

<br>
<br>

## If

Enters the logic block if the comparison <br>
of the two given values returns true.<br>
The block must be ended with the `endif` command.

### Syntax

```php
if <Value A> <Comparator> <Value B> :

    <Contents>
    
endif
```

### Example

```haskell
if $ballPosY > 62:
    change $ballVelY = 1
endif
```

<br>
