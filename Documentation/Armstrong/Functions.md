
# Functions

*Available **Armstrong** functions.*

<br>

## Define

Macro that sets a memory address to the <br>
given values before the program is executed.

### Syntax

<kbd>  define  </kbd>   <kbd>  \< Address \>  </kbd>   <kbd>  \< Value \>  </kbd>

### Example

```Armstrong
define 0xff2 33
```

<br>
<br>

## Change

Assigns the given address the supplied value.

### Syntax

<kbd>  change  </kbd>   <kbd>  \< Address \>  </kbd>   <kbd>  =  </kbd>   <kbd>  \< Value \>  </kbd>

### Example

```Armstrong
change $cursorChar = 8
```

<br>
<br>

## Add / Subtract / Multiply / Divide

Combines the two given values depending on the <br>
operator and saves the result at the given address.

### Syntax

<kbd>  \< Operator \>  </kbd>   <kbd>  \< Value A \>  </kbd>   <kbd>  ,  </kbd>   <kbd>  \< Value B \>  </kbd>   <kbd>  ->  </kbd>   <kbd>  \< Address \>  </kbd>

### Example

```Armstrong
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

<kbd>  goto  </kbd>   <kbd>  \< Address \>  </kbd>

### Example

```Armstrong
goto #mainLoop
```

<br>
<br>

## Goto If

Jumps to the given address or label if the <br>
comparison of the two values return true.

### Syntax

<kbd>  gotoif  </kbd>   <kbd>  \< Value A \>  </kbd>   <kbd>  \< Comparator \> </kbd>   <kbd>  \< Value B \>  </kbd>   <kbd>  ,  </kbd>   <kbd>  \< Address \>  </kbd>

### Example

```Armstrong
gotoif $pixPos < 4095 , #colorScreen
gotoif $pixPos == 4095 , #colorScreen
```

<br>
<br>

## If

Enters the logic block if the comparison <br>
of the two given values returns true.

### Syntax

<kbd>  if  </kbd>   <kbd>  \< Value A \>  </kbd>   <kbd>  \< Comparator \> </kbd>   <kbd>  \< Value B \>  </kbd>   <kbd>  :  </kbd>

      <kbd>  \< Logic Block \>  </kbd>

<kbd>  endif  </kbd>

### Example

```Armstrong
if $ballPosY > 62 :
    change $ballVelY = 1
endif
```

<br>
