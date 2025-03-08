,
, define:  '$expLoc' as '53500'
set 16382 53500
,
, change:  '$WIDTH' to '108'

ldia 108


stlge
here 16383
,
, change:  '$HEIGHT' to '108'

ldia 108


stlge
here 16384
,
, change:  '$pixOffset' to '53871'

ldw
here 53871


stlge
here 16385
,
, change:  '$charOffset' to '53546'

ldw
here 53546


stlge
here 16386
,
, change:  '$slowBallSpeed' to '0'

ldia 0


stlge
here 16387
,
, change:  '$slowPaddleSpeed' to '0'

ldia 0


stlge
here 16388
,
, change:  '*[1]$expLoc' to '168'

ldib 168

ldlge
here 16382

bnk 1
staout
bnk 0
,
, change:  '$key' to '168'

ldia 168


stlge
here 16389
,
, change:  '$LEFTSCOREPOS' to '53568'

ldw
here 53568


stlge
here 16390
,
, change:  '$RIGHTSCOREPOS' to '53578'

ldw
here 53578


stlge
here 16391
,
, change:  '$PADDLECHAR' to '11'

ldia 11


stlge
here 16392
,
, change:  '$NETCHAR' to '65'

ldia 65


stlge
here 16393
,
, change:  '$W' to '35'

ldia 35


stlge
here 16394
,
, change:  '$S' to '31'

ldia 31


stlge
here 16395
,
, change:  '$UP' to '71'

ldia 71


stlge
here 16396
,
, change:  '$DOWN' to '72'

ldia 72


stlge
here 16397
,
, change:  '$movingLeft' to '0'

ldia 0


stlge
here 16398
,
, change:  '$movingRight' to '0'

ldia 0


stlge
here 16399
,
, change:  '$pixelPosition' to '0'

ldia 0


stlge
here 16400
,
, change:  '$lcharPosition' to '0'

ldia 0


stlge
here 16401
,
, change:  '$rcharPosition' to '0'

ldia 0


stlge
here 16402
,
, change:  '$temp' to '0'

ldia 0


stlge
here 16403
,
, change:  '$tempB' to '0'

ldia 0


stlge
here 16404

,
, == #start ==
,
, change:  '$ballPosX' to '54'

ldia 54


stlge
here 16405
,
, change:  '$ballPosY' to '54'

ldia 54


stlge
here 16406
,
, change:  '$ballVelY' to '1'

ldia 1


stlge
here 16407
,
, change:  '$ballVelX' to '1'

ldia 1


stlge
here 16408
,
, change:  '$leftScore' to '0'

ldia 0


stlge
here 16409
,
, change:  '$rightScore' to '0'

ldia 0


stlge
here 16410
,
, change:  '$lPaddleY' to '9'

ldia 9


stlge
here 16411
,
, change:  '$rPaddleY' to '9'

ldia 9


stlge
here 16412

,
, == #netDrawLoop ==
,
, mult'  '$temp' with '18' into '$tempB'
ldib 18

ldlge
here 16403
mult


stlge
here 16404
,
, add'  '9' with '$tempB' into '$tempB'


ldlge
here 16404
swp

ldia 9
add


stlge
here 16404
,
, add'  '$tempB' with '$charOffset' into '$tempB'


ldlge
here 16386
swp

ldlge
here 16404
add


stlge
here 16404
,
, change:  '*[1]$tempB' to '$NETCHAR'


ldlge
here 16393
swp

ldlge
here 16404

bnk 1
staout
bnk 0
,
, add'  '$temp' with '1' into '$temp'
ldib 1

ldlge
here 16403
add


stlge
here 16403
,
, gotoif:   '$temp < 18' -> '#netDrawLoop'
ldib 18

ldlge
here 16403

sub
jmpz

here 145
jmpc

here 145
jmp
here 100

,
, == #update ==
,
, gotoif:   '$slowPaddleSpeed != 14000' -> '#__IF-ID1__'
ldw
here 14000
swp

ldlge
here 16388

sub
jmpz

here 155
jmp
here 453
,
, change:  '$key' to '*[1]$expLoc'


ldlge
here 16382

bnk 1
ldain
bnk 0


stlge
here 16389
,
, define:  '$mask' as '32768'
set 16413 32768
,
, and'  '$key' with '$mask' into '$pressed'


ldlge
here 16413
swp

ldlge
here 16389
and


stlge
here 16414
,
, define:  '$big' as '32767'
set 16415 32767
,
, and'  '$key' with '$big' into '$key'


ldlge
here 16415
swp

ldlge
here 16389
and


stlge
here 16389
,
, change:  '$slowPaddleSpeed' to '0'

ldia 0


stlge
here 16388
,
, gotoif:   '$pressed != $mask' -> '#__IF-ID3__'

ldlge
here 16413
swp

ldlge
here 16414

sub
jmpz

here 191
jmp
here 217
,
, gotoif:   '$key != $W' -> '#__IF-ID21__'

ldlge
here 16394
swp

ldlge
here 16389

sub
jmpz

here 201
jmp
here 204
,
, change:  '$movingLeft' to '2'

ldia 2


stlge
here 16398

,
, == #__IF-ID21__ ==
,
, gotoif:   '$key != $S' -> '#__IF-ID22__'

ldlge
here 16395
swp

ldlge
here 16389

sub
jmpz

here 214
jmp
here 217
,
, change:  '$movingLeft' to '1'

ldia 1


stlge
here 16398

,
, == #__IF-ID22__ ==

,
, == #__IF-ID3__ ==
,
, gotoif:   '$pressed == $mask' -> '#__IF-ID4__'

ldlge
here 16413
swp

ldlge
here 16414

sub
jmpz
here 251
,
, gotoif:   '$key != $W' -> '#__IF-ID23__'

ldlge
here 16394
swp

ldlge
here 16389

sub
jmpz

here 235
jmp
here 238
,
, change:  '$movingLeft' to '0'

ldia 0


stlge
here 16398

,
, == #__IF-ID23__ ==
,
, gotoif:   '$key != $S' -> '#__IF-ID24__'

ldlge
here 16395
swp

ldlge
here 16389

sub
jmpz

here 248
jmp
here 251
,
, change:  '$movingLeft' to '0'

ldia 0


stlge
here 16398

,
, == #__IF-ID24__ ==

,
, == #__IF-ID4__ ==
,
, gotoif:   '$movingLeft != 2' -> '#__IF-ID5__'
ldib 2

ldlge
here 16398

sub
jmpz

here 259
jmp
here 271
,
, sub'  '$lPaddleY' with '1' into '$lPaddleY'
ldib 1

ldlge
here 16411
sub


stlge
here 16411
,
, change:  '*[1]$lcharPosition' to '0'

ldib 0

ldlge
here 16401

bnk 1
staout
bnk 0

,
, == #__IF-ID5__ ==
,
, gotoif:   '$movingLeft != 1' -> '#__IF-ID6__'
ldib 1

ldlge
here 16398

sub
jmpz

here 279
jmp
here 291
,
, add'  '$lPaddleY' with '1' into '$lPaddleY'
ldib 1

ldlge
here 16411
add


stlge
here 16411
,
, change:  '*[1]$lcharPosition' to '0'

ldib 0

ldlge
here 16401

bnk 1
staout
bnk 0

,
, == #__IF-ID6__ ==
,
, gotoif:   '$pressed != $mask' -> '#__IF-ID7__'

ldlge
here 16413
swp

ldlge
here 16414

sub
jmpz

here 301
jmp
here 327
,
, gotoif:   '$key != $UP' -> '#__IF-ID25__'

ldlge
here 16396
swp

ldlge
here 16389

sub
jmpz

here 311
jmp
here 314
,
, change:  '$movingRight' to '2'

ldia 2


stlge
here 16399

,
, == #__IF-ID25__ ==
,
, gotoif:   '$key != $DOWN' -> '#__IF-ID26__'

ldlge
here 16397
swp

ldlge
here 16389

sub
jmpz

here 324
jmp
here 327
,
, change:  '$movingRight' to '1'

ldia 1


stlge
here 16399

,
, == #__IF-ID26__ ==

,
, == #__IF-ID7__ ==
,
, gotoif:   '$pressed == $mask' -> '#__IF-ID8__'

ldlge
here 16413
swp

ldlge
here 16414

sub
jmpz
here 361
,
, gotoif:   '$key != $UP' -> '#__IF-ID27__'

ldlge
here 16396
swp

ldlge
here 16389

sub
jmpz

here 345
jmp
here 348
,
, change:  '$movingRight' to '0'

ldia 0


stlge
here 16399

,
, == #__IF-ID27__ ==
,
, gotoif:   '$key != $DOWN' -> '#__IF-ID28__'

ldlge
here 16397
swp

ldlge
here 16389

sub
jmpz

here 358
jmp
here 361
,
, change:  '$movingRight' to '0'

ldia 0


stlge
here 16399

,
, == #__IF-ID28__ ==

,
, == #__IF-ID8__ ==
,
, gotoif:   '$movingRight != 2' -> '#__IF-ID9__'
ldib 2

ldlge
here 16399

sub
jmpz

here 369
jmp
here 381
,
, sub'  '$rPaddleY' with '1' into '$rPaddleY'
ldib 1

ldlge
here 16412
sub


stlge
here 16412
,
, change:  '*[1]$rcharPosition' to '0'

ldib 0

ldlge
here 16402

bnk 1
staout
bnk 0

,
, == #__IF-ID9__ ==
,
, gotoif:   '$movingRight != 1' -> '#__IF-ID10__'
ldib 1

ldlge
here 16399

sub
jmpz

here 389
jmp
here 401
,
, add'  '$rPaddleY' with '1' into '$rPaddleY'
ldib 1

ldlge
here 16412
add


stlge
here 16412
,
, change:  '*[1]$rcharPosition' to '0'

ldib 0

ldlge
here 16402

bnk 1
staout
bnk 0

,
, == #__IF-ID10__ ==
,
, mult'  '$lPaddleY' with '18' into '$temp'
ldib 18

ldlge
here 16411
mult


stlge
here 16403
,
, add'  '$charOffset' with '$temp' into '$lcharPosition'


ldlge
here 16403
swp

ldlge
here 16386
add


stlge
here 16401
,
, change:  '*[1]$lcharPosition' to '$PADDLECHAR'


ldlge
here 16392
swp

ldlge
here 16401

bnk 1
staout
bnk 0
,
, mult'  '$rPaddleY' with '18' into '$temp'
ldib 18

ldlge
here 16412
mult


stlge
here 16403
,
, add'  '17' with '$temp' into '$temp'


ldlge
here 16403
swp

ldia 17
add


stlge
here 16403
,
, add'  '$charOffset' with '$temp' into '$rcharPosition'


ldlge
here 16403
swp

ldlge
here 16386
add


stlge
here 16402
,
, change:  '*[1]$rcharPosition' to '$PADDLECHAR'


ldlge
here 16392
swp

ldlge
here 16402

bnk 1
staout
bnk 0
,
, present video buffer
vbuf

,
, == #__IF-ID1__ ==

,
, == #skipPaddles ==
,
, gotoif:   '$slowBallSpeed != 10000' -> '#__IF-ID2__'
ldw
here 10000
swp

ldlge
here 16387

sub
jmpz

here 463
jmp
here 741
,
, change:  '$slowBallSpeed' to '0'

ldia 0


stlge
here 16387
,
, gotoif:   '$ballVelX != 0' -> '#__IF-ID11__'
ldib 0

ldlge
here 16408

sub
jmpz

here 474
jmp
here 480
,
, sub'  '$ballPosX' with '1' into '$ballPosX'
ldib 1

ldlge
here 16405
sub


stlge
here 16405

,
, == #__IF-ID11__ ==
,
, gotoif:   '$ballVelX != 1' -> '#__IF-ID12__'
ldib 1

ldlge
here 16408

sub
jmpz

here 488
jmp
here 494
,
, add'  '$ballPosX' with '1' into '$ballPosX'
ldib 1

ldlge
here 16405
add


stlge
here 16405

,
, == #__IF-ID12__ ==
,
, gotoif:   '$ballVelY != 0' -> '#__IF-ID13__'
ldib 0

ldlge
here 16407

sub
jmpz

here 502
jmp
here 508
,
, sub'  '$ballPosY' with '1' into '$ballPosY'
ldib 1

ldlge
here 16406
sub


stlge
here 16406

,
, == #__IF-ID13__ ==
,
, gotoif:   '$ballVelY != 1' -> '#__IF-ID14__'
ldib 1

ldlge
here 16407

sub
jmpz

here 516
jmp
here 522
,
, add'  '$ballPosY' with '1' into '$ballPosY'
ldib 1

ldlge
here 16406
add


stlge
here 16406

,
, == #__IF-ID14__ ==
,
, gotoif:   '$ballPosX != 0' -> '#__IF-ID15__'
ldib 0

ldlge
here 16405

sub
jmpz

here 530
jmp
here 555
,
, change:  '$ballPosX' to '54'

ldia 54


stlge
here 16405
,
, change:  '$ballPosY' to '54'

ldia 54


stlge
here 16406
,
, change:  '$ballVelX' to '1'

ldia 1


stlge
here 16408
,
, add'  '$rightScore' with '1' into '$rightScore'
ldib 1

ldlge
here 16410
add


stlge
here 16410
,
, gotoif:   '$rightScore != 10' -> '#__IF-ID29__'
ldib 10

ldlge
here 16410

sub
jmpz

here 553
jmp
here 555
,
, goto:    'goto' '76'
jmp
here 76

,
, == #__IF-ID29__ ==

,
, == #__IF-ID15__ ==
,
, gotoif:   '$ballPosX != 107' -> '#__IF-ID16__'
ldib 107

ldlge
here 16405

sub
jmpz

here 563
jmp
here 588
,
, change:  '$ballPosX' to '54'

ldia 54


stlge
here 16405
,
, change:  '$ballPosY' to '54'

ldia 54


stlge
here 16406
,
, change:  '$ballVelX' to '0'

ldia 0


stlge
here 16408
,
, add'  '$leftScore' with '1' into '$leftScore'
ldib 1

ldlge
here 16409
add


stlge
here 16409
,
, gotoif:   '$leftScore != 10' -> '#__IF-ID30__'
ldib 10

ldlge
here 16409

sub
jmpz

here 586
jmp
here 588
,
, goto:    'goto' '76'
jmp
here 76

,
, == #__IF-ID30__ ==

,
, == #__IF-ID16__ ==
,
, gotoif:   '$ballPosY != 0' -> '#__IF-ID17__'
ldib 0

ldlge
here 16406

sub
jmpz

here 596
jmp
here 599
,
, change:  '$ballVelY' to '1'

ldia 1


stlge
here 16407

,
, == #__IF-ID17__ ==
,
, gotoif:   '$ballPosY != 107' -> '#__IF-ID18__'
ldib 107

ldlge
here 16406

sub
jmpz

here 607
jmp
here 610
,
, change:  '$ballVelY' to '0'

ldia 0


stlge
here 16407

,
, == #__IF-ID18__ ==
,
, gotoif:   '$ballPosX >= 5' -> '#__IF-ID19__'
ldib 5

ldlge
here 16405

sub
jmpz
here 655
jmpc
here 655
,
, mult'  '$lPaddleY' with '6' into '$temp'
ldib 6

ldlge
here 16411
mult


stlge
here 16403
,
, add'  '$temp' with '6' into '$tempB'
ldib 6

ldlge
here 16403
add


stlge
here 16404
,
, gotoif:   '$ballPosY >= $tempB' -> '#__IF-ID31__'

ldlge
here 16404
swp

ldlge
here 16406

sub
jmpz
here 655
jmpc
here 655
,
, gotoif:   '$ballPosY <= $temp' -> '#__IF-ID33__'

ldlge
here 16403
swp

ldlge
here 16406

sub
jmpz
here 655
jmpc

here 652
jmp
here 655
,
, change:  '$ballVelX' to '1'

ldia 1


stlge
here 16408

,
, == #__IF-ID33__ ==

,
, == #__IF-ID31__ ==

,
, == #__IF-ID19__ ==
,
, gotoif:   '$ballPosX <= 98' -> '#__IF-ID20__'
ldib 98

ldlge
here 16405

sub
jmpz
here 702
jmpc

here 665
jmp
here 702
,
, mult'  '$rPaddleY' with '6' into '$temp'
ldib 6

ldlge
here 16412
mult


stlge
here 16403
,
, add'  '$temp' with '6' into '$tempB'
ldib 6

ldlge
here 16403
add


stlge
here 16404
,
, gotoif:   '$ballPosY >= $tempB' -> '#__IF-ID32__'

ldlge
here 16404
swp

ldlge
here 16406

sub
jmpz
here 702
jmpc
here 702
,
, gotoif:   '$ballPosY <= $temp' -> '#__IF-ID34__'

ldlge
here 16403
swp

ldlge
here 16406

sub
jmpz
here 702
jmpc

here 699
jmp
here 702
,
, change:  '$ballVelX' to '0'

ldia 0


stlge
here 16408

,
, == #__IF-ID34__ ==

,
, == #__IF-ID32__ ==

,
, == #__IF-ID20__ ==
,
, change:  '*[1]$pixelPosition' to '0'

ldib 0

ldlge
here 16400

bnk 1
staout
bnk 0
,
, mult'  '$ballPosY' with '$WIDTH' into '$temp'


ldlge
here 16383
swp

ldlge
here 16406
mult


stlge
here 16403
,
, add'  '$temp' with '$ballPosX' into '$temp'


ldlge
here 16405
swp

ldlge
here 16403
add


stlge
here 16403
,
, add'  '$temp' with '$pixOffset' into '$pixelPosition'


ldlge
here 16385
swp

ldlge
here 16403
add


stlge
here 16400
,
, change:  '*[1]$pixelPosition' to '65535'

ldw
here 65535
swp

ldlge
here 16400

bnk 1
staout
bnk 0
,
, present video buffer
vbuf

,
, == #__IF-ID2__ ==
,
, add'  '$leftScore' with '39' into '$temp'
ldib 39

ldlge
here 16409
add


stlge
here 16403
,
, change:  '*[1]$LEFTSCOREPOS' to '$temp'


ldlge
here 16403
swp

ldlge
here 16390

bnk 1
staout
bnk 0
,
, add'  '$rightScore' with '39' into '$temp'
ldib 39

ldlge
here 16410
add


stlge
here 16403
,
, change:  '*[1]$RIGHTSCOREPOS' to '$temp'


ldlge
here 16403
swp

ldlge
here 16391

bnk 1
staout
bnk 0
,
, add'  '$slowBallSpeed' with '1' into '$slowBallSpeed'
ldib 1

ldlge
here 16387
add


stlge
here 16387
,
, add'  '$slowPaddleSpeed' with '1' into '$slowPaddleSpeed'
ldib 1

ldlge
here 16388
add


stlge
here 16388
,
, goto:    'goto' '145'
jmp
here 145
