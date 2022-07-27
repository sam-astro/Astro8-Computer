### Armstrong:
```
// Use hex values (0xff) when referring to addresses, and decimal (213) for a new immediate integer

@A - A register
@B - B register
@C - C register
@EX - Expansion port

$                              - '$' symbol refers to a variable containing an integer (address in memory)
@                              - '@' symbol refers to a register, ie. ('@A' for register 'A')
#                              - '#' symbol refers to a label, which is a point the program can jump to ie. ('#main') (address in program counter)
define <addr> <val>            - Assembler defines <addr> equal to <val>. Sets memory values before the program is executed, then is removed.
change <addr> = <val>          - Change <addr> to <val> at any time
add <valA>,<valB> -> <addr>    - Add the values <valA> and <valB>, then store the result in <addr>
sub <valA>,<valB> -> <addr>    - Subtract the values <valA> and <valB> (valA - valB), then store the result in <addr>
mult <valA>,<valB> -> <addr>   - Multiply the values <valA> and <valB>, then store the result in <addr>
div <valA>,<valB> -> <addr>    - Divide the values <valA> and <valB> (valA / valB), then store the result in <addr>
goto <addr>                    - Jumps to the given address or label
gotoif <valA><C><valB>,<addr>  - Jumps to <addr>, given the logic relationship between <valA> and <valB> given a comparer <C>, ie. (jmpc 0x12==4,0x0)
if <valA><C><valB>             - Continues if the logic relationship between <valA> and <valB> given a comparer <C> is true, ie. (if 0x12==4)
endif                          - Marks the ending of the contents of an if statement
```