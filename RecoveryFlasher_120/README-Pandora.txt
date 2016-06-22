HC Recovery Flasher - Pandora README
-----------------------------------------

Since 1.20 the Recovery Flasher also comes build as an .ELF binary to 
be used on a Pandora stick.

*** This is for Pandora Sticks with DDCv5 or earlier!
    (prefferabely DDCv4, that's what used before v6 ;-) )
    On DDCv6 this will NOT work, but on there you can use the EBOOT
    version, it works fine from DDCv6.

This way you can use the CFW flashing functions of this tool from a 
Pandora boot as well, to "unchain" or unbrick or whatever.

The backup/restore functions are NOT available in "Pandora Mode".
The app launching function may not work for all apps as well.


Installation
========================================

There are two ways of installing this - but both require a set up and 
working DDC magicstick:

The preffered way is to launch the RFLASH.ELF via jas0nuk's ELF-Menu 
(ELF-Menu itself can be easiely installed with "Pandora Installer").
Copy the RFLASH.ELF to /elf on your magicstick.
Also copy the original Recovery Flasher's EBOOT.PBP there!
Last but not least, copy all (propperly renamed) Sony updaters of the 
FWs you plan to install there as well (i.e. 371.PBP, 401.PBP).

The other way would be to use it as a ressurection.elf replacement.
Installation is pretty much the same as above, only that you copy 
everything to /kd instead and you rename the RFLASH.ELF to 
ressurection.elf before copying it over.
