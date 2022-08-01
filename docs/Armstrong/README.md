# Armstrong

*The **Armstrong** syntax guide.*

<br>
<br>

[![Button Functions]][Functions]

<br>
<br>

## Hex

Use hexadecimal values for **Addresses**.

```Armstrong
0xff
```

<br>
<br>

## Decimal

Use decimal values for **Immediate Integers**.

```Armstrong
213
```

<br>
<br>

## Comments

### Syntax

<kbd>  //  </kbd> <kbd>  \< You Comment Text \>  </kbd>

### Example

```armstrong
// This is a comment
```

<br>
<br>

## Registers

### Syntax

<kbd>  @  </kbd> <kbd>  \< Register Name \>  </kbd>

### Available

| Name | Example |
|:----:|:-------:|
| ***Register A - C*** | `@A`  `@B`  `@C`
| ***Expansion Port*** | `@EX`

<br>
<br>

## Labels

A label is a place in the program you can jump to. <br>
*You can also see it as an address in the program.*

### Syntax

<kbd>  #  </kbd> <kbd>  \< Label Name \>  </kbd>

### Example

```Armstrong
#start
```

<br>
<br>

## Variables

Variable can contain an an integers only. <br>
*You can think of them as addresses in memory.*

### Syntax

<kbd>  $  </kbd> <kbd>  \< Variable Name \>  </kbd>

### Example

```Armstrong
$ballVelocity
```

<br>
<br>

## Comparator

Some functions compare two values and <br>
determine if their relationship is valid.

### Types

| Symbol | Meaning 
|:------:|:--------
| `<`    | Is **A** less than **B**
| `>`    | Is **A** greater than **B**
| `==`   | Are **A** and **B** equal
| `!=`   | Are **A** and **B** not equal

<br>


<!----------------------------------------------------------------------------->

[Functions]: Functions.md


<!---------------------------------[ Buttons ]--------------------------------->

[Button Functions]: https://img.shields.io/badge/Functions-0288D1?style=for-the-badge&logoColor=white&logo=Betfair