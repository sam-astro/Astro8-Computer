---
layout : default
grand_parent: Programming
parent: Armstrong
title : Commands
---

# Commands

*Available **Armstrong** functions.*

<br>

## Define

Macro that sets a memory address to the <br>
given values before the program is executed.

### Syntax

```
define 〈 Ａｄｄｒｅｓｓ 〉 〈 Ｖａｌｕｅ 〉
```

### Example

```
define 0xff2 33
```

<br>
<br>

## Change

Assigns the given address the supplied value.

### Syntax

```
change 〈 Ａｄｄｒｅｓｓ 〉 = 〈 Ｖａｌｕｅ 〉
```

### Example

```
change $cursorChar = 8
```

<br>
<br>

## Add / Subtract / Multiply / Divide

Combines the two given values depending on the <br>
operator and saves the result at the given address.

### Syntax

```
〈 Ｏｐｅｒａｔｏｒ 〉 〈 Ｖａｌｕｅ　Ａ 〉 , 〈 Ｖａｌｕｅ　Ｂ 〉 -> 〈 Ａｄｄｒｅｓｓ 〉
```

### Example

```
add $A , $B -> $C  //  A + B
mul $A , $B -> $C  //  A * B
sub $A , $B -> $C  //  A - B
div $A , $B -> $C  //  A / B
```

<br>
<br>

## Goto

Jumps to the given address or label.

### Syntax

```
goto 〈 Ａｄｄｒｅｓｓ 〉
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
gotoif 〈 Ｖａｌｕｅ　Ａ 〉 〈 Ｃｏｍｐａｒａｔｏｒ 〉 〈 Ｖａｌｕｅ　Ｂ 〉 , 〈 Ａｄｄｒｅｓｓ 〉
```

### Example

```
gotoif $pixPos < 4095 , #colorScreen
gotoif $pixPos == 4095 , #colorScreen
```

<br>
<br>

## If

Enters the logic block if the comparison <br>
of the two given values returns true.

### Syntax

```
if 〈 Ｖａｌｕｅ　Ａ 〉 〈 Ｃｏｍｐａｒａｔｏｒ 〉 〈 Ｖａｌｕｅ　Ｂ 〉 :

    〈 Ｌｏｇｉｃ　Ｂｌｏｃｋ 〉
    
endif
```

### Example

```
if $ballPosY > 62:
    change $ballVelY = 1
endif
```

<br>
