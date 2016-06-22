
   H E L L C A T s   R e c o v e r y   F l a s h e r
 =======================================================
   * recover semi-bricks * up-/downgrade your CFW *
   * flash *ANY* CFW directely to 3.71-M33 *
    all without Pandora
 =======================================================


What is this? And why?
========================================

Short version:
--------------------
- Flashes a fresh M33 CFW (3.71, 4.01)
  for recovering semi-bricks or to up-/downgrade CFW
- does not use Pandora, runns from the recovery menu or the XMB
- does not use (as in launch) the original Sony Updater (only extracts 
  all required files from the original .PBP)
- runns fine on CFWs lower than 3.52-M33-3 - handy for upgrading older 
  FWs w/o the "need to go 3.52, then 3.52-3, then higher" odyssey :-)
- runns fine from TimeMachine (3.40-OE on Fat and 3.60-M33 on Slim)
- can backup the FW flash and restore selected parts (XMB theme, 
  settings, PSN activation)


Pandora Support
========================================

Since version 1.20 the recovery flasher also comes with an .ELF binary 
that can be used from a DDC Pandora stick, so you can use the CFW 
flashing functions of it from a Pandora boot.

For details on how to install and use it, see the readme in the 
Pandora folder of this archive.


Long version: :)
--------------------
Back in the 1.50 based CFW days (up to 3.52-M33) the option "Launch 
EBOOT at /PSP/GAME/RECOVERY/" was a very handy thing for recovering 
semi-bricks or downgrading your PSP - in short, to repair or 
completely re-flash your PSP.

Since the 3.xx (or higher) based CFWs this option became obsolete due 
to missing apps that could run and do their job from there.

Here comes the rescue ;-)

Designed as a native 3.xx+ app, to run from the recovery menu's option
from recent CFWs on Fat and Slim PSPs - however, you can perfectly run it
from the XMB as well.
On 1.50 based CFWs (3.52 and lower) you must run it from the XMB,
since the recovery menu on those CFWs launches the EBOOT in 1.50 kernel,
but it uses systen calls not available from there.

You can perform the following actions:

- Flash a fresh 3.71-M33 (-1) to recover a semi-brick or to downgrade.
- Make a backup of all FW related flash content (filewise, flash0-3, 
i.e. FW, settings, PSN store activation, e.t.c., it's not a NAND dump!)
- Restore various parts of the FW flash (VSH/XMB theme, settings, PSN 
store activation....)

When installing a fresh 3.71 you can choose to reset all settings as 
well, or to keep them if you know there's nothing wrong with them.
So the recovered/downgraded PSP can still have all your settings! :-)

All of theese functions also work from a 3.60 booted via TimeMachine!
Even from a 3.40-OE.

Why 3.71? Why not a lower or higher FW?
========================================

One purpose of this app is to downgrade the CFW of the PSP.
I define downgrading much as "go as low as you can" and 3.71 is the 
lowest firmware that runns on both PSPs, Fat and Slim.

From there then, you have the freedom of upgrading to any other FW 
you desire - thus no higher one for this.

However:
*** Since 1.20 you can also flash 4.01-M33-2 :-)


Installation
========================================

Painless and easy:
Copy the RECOVERY folder from the archive to /PSP/GAME/ on your 
memstick, so that you get a /PSP/GAME/RECOVERY/ folder containing the 
apps EBOOT on your stick.
Now copy the official Sony 3.71 and/or 4.01 Update EBOOT, renamed
as 371.PBP (401.PBP), into the RECOVERY folder as well.

*** NOTE ***
The Recovery Flasher MUST be installed in /PSP/GAME/RECOVERY!
Do not rename it's folder or it will fail loading!
If you have other apps you like to use from the recovery menu, you
can do so with Recovery Flasher's option "Launch another app" (see
below)

Done :-)


Usage
========================================

After launching the app, either from the recovery menu or via the XMB 
you'll eventually get into the mainmenu.

Everything should be pretty selfexplanatory, you can flash a new 
firmware, make a complete flash0 to flash2 backup or restore certain 
things from your backup.

The option "Launch another app" will give you a list of the games/apps
you have in /PSP/GAME on your memstick.
You can then choose one and run it directely from Recovery Flasher.
Usefull to use some other apps on semibricks when you can't use the
XMB to launch them, or for those people who already have a preffered
app they use from the recovery menu but want to install the Flasher
as well.


What else?
========================================

Well, that's pretty much it.

Here's some additional credits that should be noted:

- PSAR extraction is based on the M33 NEWPSARDumper - with lots of 
modifications though.... (libpsardumper.prx and pspdecrypt.prx are 
used for the actual extraction and crypto parts)

- IPL flashing routines are currently by DAX (iplupdate.prx)
(planning on getting my own code working for that part....)

- I have to thank the following people VERY MUCH for their hints, help 
and patient answers to all my questions - without those, the creation 
of this program would have taken a year longer (or two ;-)):
* JumpR * cory1492 * jas0nuk * all the other devs on LAN.ST *


I hope you enjoy using this as much as I did making it :-)

Leave comments, bug reports, or whatever in the release threads at 
either eXophase.com or the qj.net forums.


This program has been tested by myself to successfully flash
a fresh 3.71-M33-4 and a 4.01-M33-2 on the following setups:

3.02-OE *) on Fat PSP
3.40-OE    via TimeMachine on Fat PSP
3.60-M33   via TimeMachine on Slim PSP
3.71-M33   on Fat and Slim PSP
4.01-M33   on Fat and Slim PSP
4.01-M33   started via DDCv6 (kinda pointless to install a 4.01 then,
           but hey, can't hurt to try ;))

*) Note: On lower CFWs (early OEs, mostly) you may get warnings during
         3.71 installation regaring assignments of flash1:
         This is normal under theese FW versions, no need to worry.
         The program will disable formatting flash1 in that case, so
         everything should be fine.


So long! :-)
---==> HELLCAT <==---
