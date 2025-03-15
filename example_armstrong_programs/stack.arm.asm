,
, label
#loop:
,
, change:  '$charval' to '7'

ldia 7


stlge
here 60000
,
, gotoif:   '$charval != 168' -> '#loop'
ldib 168

ldlge
here 60000

sub
jmpz

here 12
jmp
here #loop
