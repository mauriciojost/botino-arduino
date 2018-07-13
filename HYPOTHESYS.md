# 

- Wiring? 
- Problem with read jsons from dweet.io

## MAYBE CULPRIT

- PC with hidden settings?
- Interrupts?
- Why so many Jul 11 20:18:07 MA E > GET:-1 connection refused???? ALL!
- Every time there is a connection refused and the body not read the httpclient grows 


## PROBABLY NOT CULPRIT

- USB cable? -> tried another one
- HTTP timeout exists? ignored? -> Tried setting up HTTP timeout but problem persists
- Broken platformio -> removed all ~/.platformio and downloaded it again
- Version of platformio changed? -> 3.5.3 since always
- Flash memory config not erased?  -> Loaded firmware in old chip and problem was inserted
- WDT issue (code changed and persistence)? -> Loaded version with no WDT code line in old chip and problem was inserted

## SURE NOT CULPRIT

- Burnt chip? -> Tried with another chip and problem reproduced immediately
- Code change? -> Tried with older versions and problem reproduces
- Code change breaking chip? -> Tried with older versions on another chip

## What we know

- All http fail!!!!!!!! 1st priority! Probably related to the issue!!!
- Try to reproduce in the simplest version ever in branch simplest
- Using -1 as log level makes the system crash
- Using frequent propsync fires it

*It does not crash when:*
- Disabling syncs (putting Never)
