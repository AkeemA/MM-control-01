# MM-control-01 - port for Arduino Mega + Ramps
MMU 3-axis stepper control

## BOM
Components which I used:  
NOTE: I had old Makerbot Replicator 1 clone, so I took the parts from it.  
  
- 3x - NEMA 17 motors  
  I took them from Makerbot Replicator 1 clone  
  Spec: 200 steps/rev  
  Z motor with integrated lead screw -> New selector motor  
  
- 2x - 5x120mm smooth rods  
  120mm long and 5mm in diameter for lead rods for selector. Replicator 1 had two long 5mm rods, I just cuted them to corect length.  
  
- 1x - 5x90mm smooth rod  
  90mm long and 5mm in diameter for pulley rod. Replicator 1 had two long 5mm rods, I just cuted them to corect length.  
  
- 8x - 5x16mm smooth rods  
  16mm long and 5mm in diameter for bearings rods and other joints. Replicator 1 had two long 5mm rods, I just cuted them to corect length.  
  
- 1x - lead screw nut  
  You should get one lead screw nut with your NEMA 17 motor with integrated lead screw or just with lead screw. They need to fit each other.  
  IMPORTANT: Remember that you will need to mod your selector printed part to fit your lead screw nut!  
  
- 9x - 625zz ball bearings  
  Just bearings for MMU2 body, assembly as described in official Prusa MMU2 assembly instructions  
  
- 1x - P.I.N.D.A probe or similar  
  You will need one for MMU2 selector. I had 12mm inductive sensor, so I modify my selector part to use it instead of P.I.N.D.A probe  
  
- 5x - MK8 hobbed gears  
  To push filament  
  
- 1x - 5x5 Shaft coupler  
  Needed to connect pulley motor with 5mm pulley rod (90mm long)  
  
- 1x - 12-15V, 2A Power supply  
  To power electronics and motors  
  
- 1x - Arduino Mega 2560  
  Regular Arduino Mega board  
  
- 1x - Ramps board  
  Use whatever you have. I had Ramps 1.6 plus which is nice in terms of quality and power safety but is harder to convert it to support 24V (Could be handy to connect MMU2 directly to Einsy Board)
  
- 3x - Stepper drivers  
  I am using A4988 but teoreticaly you can use whatever you want (need to support STEP, DIR, ENABLE signal).  
  IMPORTANT: Remember that I configured the source code for A4988 and motors with 200 steps/rev. If you want to use something else, you will need to reconfigure it.  
  
- 1x - ~6mm metal ball  
  To convert P.I.N.D.A probe to FINDA probe. ~6mm because I am using 12mm inductive sensor, so I modify my selector part to support it also to fit ball which I had (not sure the size could be 6mm or 7mm)  
  
- 1x - Microswitch with roller  
  Will be used on MK3 extruder side as additional sensor attached to extruder cover.  
  
- 1x - 50mm 4 OD x 1.85 ID PTFE Tubing  
  Almost impossible to get. I will try with "high quality" 4x1.90 PTFE tube... will see  
  
The rest:  
- 1 or 2 meters of regular PTFE Tubes (4mm OD x 2mm ID)  
- Prusa style squere M3 nuts  
- M3 bolts, mostly 10mm long  
- 2 springs and longer M3 screw (30mm long)  
- 10 PC4-10 PTFE connectors  
- cables to:  
  connect motors  
  connect Einsy with MMU (4 wire)  
  connect external microswitch as second filament sensor (2 wire)  

Optional:  
- 1x - 5x8 Shaft coupler  
  If you have NEMA17 motor for selector without integrated lead screw, you will need this to connect them  
  
## Building
### Arduino
Recomended version is arduino 1.8.5.  
in MM-control-01 subfolder create file version.h  
use version.h.in as template, replace ${value} with numbers or strings according to comments in template file.  
create file dirty.h with content if you are building unmodified git commit
~~~
#define FW_LOCAL_CHANGES 0
~~~
or
~~~
#define FW_LOCAL_CHANGES 1
~~~
if you have uncommitted local changes.
#### Build
click verify to build

## Flashing
#### Arduino
click Upload
