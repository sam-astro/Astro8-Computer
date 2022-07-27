### Memory Layout:
```
word 0 |                                                       .                                                  | word 65535
       | Program mem. 0 - 16382    I                           I                          Video memory 61439-65535|

0-25%    (0 - 16382)   16382 words  -  Program mem.
25-25%   (16383-16527) 144 words    -  Character mem. (contains index of character to be displayed at the corresponding location)
25-25%   (16528-16656) 128 words    -  Variable mem.

93-100%  (61439-65535) 4096 words   -  Video mem. 
```