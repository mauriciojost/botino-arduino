#POSES

## Arms poses

Move both arms to a given position each (left, then right) (A=fast, B=normal, C=slow).

Codes:
```
  A00. : Move left and right arms to respective position 0 and 0 (both down) at high speed
  ...
  A90. : Move left and right arms to respective position 9 and 0 (left arm up) at high speed
  ...
  A99. : Move left and right arms to respective position 9 and 9 (both up) at high speed

  B99. : Move left and right arms to respective position 9 and 9 (both up) at normal speed

  C99. : Move left and right arms to respective position 9 and 9 (both up) at low speed
```


## Face poses

Show a given image in the LCD.

Codes:
```
  Fw. : Face White
  Fb. : Face Black
  Fa. : Face Angry
  Fr. : Face cRazy
  Fl. : Face cLear
  Fs. : Face Smily
  FS. : Face Sad
  Fn. : Face Normal
  Fz. : Face Zleepy
  F_. : Face dimmed
  F-. : Face bright
  F0. : Face custom 0 (user provided)
  F1. : Face custom 1 (user provided)
  F2. : Face custom 2 (user provided)
  F3. : Face custom 3 (user provided)
```


## IFTTT poses

Trigger an ifttt event by its name.

Codes:
```
  I<name>. : trigger event 'name'
```

## IO poses

Turn on/off a given IO device, such as LEDS or the FAN (on = y, off = n).

Codes:
```
  Lry. : turn on (y) the Red led
  Lrn. : turn off (n) the Red led
  Lrt. : toggle (t) the Red led
  Lwy. : turn on (y) led White
  Lyy. : turn on (y) led Yellow
  Lyt. : toggle (t) led Yellow
  L*y. : turn all leds on
  L*n. : turn all leds off
  Lfy. : turn on (y) Fan
  L?.  : turn randomly all leds
```

## Message poses

Show a certain message in the LCD with a given font size.

Codes:
```
  Mc4. : show message containing current time (with font size 4)
  Mk3. : show message containing current date-time (with font size 3)
  Mp1. : show random future reading (with font size 1)
  Mq1. : show random quote (with font size 1)
  M1<msg>. : show message msg with font size 1 (user provided)
```


## Notification poses

Show a certain notification in the LCD (requires user's ACK before removal).

Codes:
```
  N<notif>. : show notification "notif"
```


## Wait poses

Wait a given number of seconds.

Codes:
```
  W1. : wait 1 second
  ...
  W9. : wait 9 seconds
```


## Misc poseS

Codes:
```
  Z. : turn all power consuming components off
```

## Composed poses

Dances and other predefined moves usable as poses.

Codes:
```
  Dn. : dance n
  Du. : dance u
  D\. : dance \
  D/. : dance /

  D0. : dance 0
  D1. : dance 1
  D2. : dance 2
  D3. : dance 3
```
