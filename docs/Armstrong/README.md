---
has_children : true
nav_order : 6
layout : default
title : Armstrong
---

# Armstrong

Armstrong is a higher level language created to make programming for the Astro-8 a better experience. Here are the syntax and rules of the langauge.

<br>
<br>

[![Button Commands]][Commands]

<br>
<br>

## Hex

Use hexadecimal values for **Addresses**.

```
0xff
```

<br>
<br>

## Decimal

Use decimal values for **Immediate Integers**.

```
213
```

<br>
<br>

## Comments

*How to add comment to your code.*

<br>

### Syntax

```
// 〈 Ｙｏｕｒ　Ｃｏｍｍｅｎｔ 〉
```

<br>

### Example

```
// This is a comment
```

<br>
<br>

## Registers

*How to address a register.*

<br>

### Syntax

```
@ 〈 Ｒｅｇｉｓｔｅｒ　Ｎａｍｅ 〉
```

<br>

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

<br>

### Syntax

```
# 〈 Ｌａｂｅｌ　Ｎａｍｅ 〉
```

<br>

### Example

```
#start
```

<br>
<br>

## Variables

Variable can contain an an integers only. <br>
*You can think of them as addresses in memory.*

<br>

### Syntax

```
$ 〈 Ｖａｒｉａｂｌｅ　Ｎａｍｅ 〉
```

<br>

### Example

```
$ballVelocity
```

<br>
<br>

## Comparator

Some functions compare two values and <br>
determine if their relationship is valid.

<br>

### Types

| Symbol | Meaning 
|:------:|:--------
| `<`    | Is **A** less than **B**
| `>`    | Is **A** greater than **B**
| `==`   | Are **A** and **B** equal
| `!=`   | Are **A** and **B** not equal

<br>


<!----------------------------------------------------------------------------->

[Commands]: Commands


<!---------------------------------[ Buttons ]--------------------------------->

[Button Commands]: https://img.shields.io/badge/Functions-0288D1?style=flat-square&logoColor=white&logo=Betfair
