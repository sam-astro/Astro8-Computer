---
has_children : true
nav_order : 5
layout : default
title : Assembly
parent: Programming
category: Programming
---

# Assembly

The assembly language, also named **Astrisc** (For Astro RISC), is the lowest-level way to code programs for the Astro-8. This document covers how to use it.

<br>

[The total list of instructions can be found here](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Instruction%20Set.html)

<br>


## Set command

Sets the specified memory location to a value during assembly, uses following format:

```c
set <Address> <Value>
```

{: .note .caps}
This command itself does not occupy a memory location, and following code will collapse to take it's place.

<br>
<br>

## Here command

Set the location of the `HERE` in memory equal to a value during assembly, uses following format:

```c
here <Value>
```

{: .note .caps}
This command itself does not occupy a memory location, and following code will collapse to take it's place.

<br>
<br>

## Labels

You can use the `#` symbol to define a label to a location in code.

```c
#firstlabel:
    lda 2
    add
#secondlabel:
    ldib 4
```

Labels are simply constants that store the address of the following instruction. For example, this is what the code above would look like without labels (I also added line numbers):

```c
0    lda 2
1    add
2    ldib 4
```

In this case, `#firstlabel` would contain the value `0`, and `#secondlabel` would contain the value `2`.

<br>
<br>

## Constants

You can use the `const` keyword to define a constant integer variable in your code. It follows the format:

```c
const <name> <integer>
```

```c
const @xVar 5
    lda $xVar
    add
const @yVar 7
    ldib $yVar
```

Each use case of this variable will simply be replaced with the integer when the code is assembled. The above example will become:

```c
0    lda 5
1    add
2    ldib 7
```

<br>
<br>

## Comments

You can add single-line comments to your code using the comma symbol `,` followed by the comment contents.

<br>

### Syntax

```
, <Comment>
```

<br>

### Example

```
, This is a comment
add
ldia 5 , This is another comment
```

<br>
<br>

## Alloc

You can use the `alloc` keyword to allocate an area in memory. It follows the format:

```c
alloc <size>
```

Example:

```c
alloc 5
lda 3
add
```

This becomes:

```c
0    here 0
1    here 0
2    here 0
3    here 0
4    here 0
5    lda 3
6    add
```

The `alloc` simply clears that area of memory for `<size>` memory locations

<br>
<br>


<!----------------------------------------------------------------------------->

[Commands]: ../../Architecture/Instruction%20Set


<!---------------------------------[ Buttons ]--------------------------------->

[Button Commands]: https://img.shields.io/badge/Commands-0288D1?style=flat-square&logoColor=white&logo=Betfair

