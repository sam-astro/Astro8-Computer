---
has_children : true
nav_order : 5
layout : default
title : Assembly
parent: Programming
---

# Assembly

The assembly language, also named **Astrisc** (For Astro RISC), is the lowest-level way to code programs for the Astro-8. This document covers how to use it, as well as best practices.

<br>

[The total list of instructions can be found here](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Instruction%20Set.html)

<br>

## Formatting style

Astrisc is case-insensitive, meaning it does not matter if you capitalize commands or not. For style purposes, it is recommended to use `lowercase` when you are programming for all characters (it is easier to type and read). Programs that are compiled from another language (such as Armstrong or Yabal) will often output in `UPPERCASE`, which is standard practice for Astrisc that is computer-generated.

{.warn .caps}
Due to the case-insensitivity, this also means variables and labels will not be seen as different with just differently capitlized letters. For example, `#THISISALABEL` will be seen the same as `#ThisIsALabel`.


## Set command

Sets the specified memory location to a value during assembly, uses following format:

```c
set <Address> <Value>
```

{: .note .caps}
This command itself does not occupy a memory location, and following code will collapse to take it's place.

<br>

## Here command

Set the location of the `HERE` in memory equal to a value during assembly, uses following format:

```c
here <Value>
```

{: .note .caps}
This command itself does not occupy a memory location, and following code will collapse to take it's place.

<br>

## Labels

You can use the `#` symbol to define a label to a location in code.

```c
#firstlabel:
    lda 2
    add
#secondlabel:
    jmp #firstlabel
```

Labels are simply constants that store the address of the following instruction. For example, this is what the code above would look like without labels (I also added line numbers):

```c
0    lda 2
1    add
2    jmp 0
```

In this case, `#firstlabel` would contain the value `0`, and `#secondlabel` would contain the value `2`.

{.note .caps}
If there is a clear section of code the label is defined for, rather than a simple jump, it is better to make that syntactically clear by indenting with a tab or 4 spaces, as in the above syntax example.

<br>

## Constants

You can use the `const` keyword to define a constant integer variable in your code. It follows the format:

```c
const <name> <integer>
```

{.note .caps}
In modern Astrisc, it is best practice to use constants and labels if at all possible, and to avoid hard-coding line numbers and such into your program.

```c
const @xVar 5  ,define variable

lda $xVar  ,use variable
add

const @yVar 7  ,define another variable

ldib $yVar  ,use the other variable
```

Each use case of this variable will simply be replaced with the integer when the code is assembled. The above example will become:

```c
0    lda 5
1    add
2    ldib 7
```

<br>

## Comments

You can add single-line comments to your code using the comma symbol `,` followed by the comment contents.

<br>

### Syntax

```
,<Comment>
```

### Example

```
,This is a comment
add
ldia 5  ,This is another comment
```

{.note .caps}
Due to the fact the comma `,` character us used here, and is often used in other languages to separate values, the best way to use these is by placing the comment *directly after* the character like `,<comment>`, and to place ***2*** space characters `  ` before it if there is already a command. Such as `add  ,Comment`.

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

{.note .caps}
`alloc 0` would remove the command, and not place any `here 0` commands.

<br>


<!----------------------------------------------------------------------------->

[Commands]: ../../Architecture/Instruction%20Set


<!---------------------------------[ Buttons ]--------------------------------->

[Button Commands]: https://img.shields.io/badge/Commands-0288D1?style=flat-square&logoColor=white&logo=Betfair

