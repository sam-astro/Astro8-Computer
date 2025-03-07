,
, inline assembly

jmp
here #main
, STD Library:
const @__CHAR_MEM_ADDR__ 53546
const @__PIXEL_MEM_ADDR__ 53870
#__CHAR_PTR__:
here 0
#printf:
, End STD Library
,
, define:  '$call_stack_ptr' as '65535'
set 60000 65535
,
, define:  '$call_stack_min' as '65023'
set 59999 65023
,
, define:  '$stack_ptr' as '65022'
set 59998 65022
,
, define:  '$stack_min' as '64510'
set 59997 64510

,
, label
#return:
