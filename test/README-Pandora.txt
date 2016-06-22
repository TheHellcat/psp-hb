HC Recovery Flasher - Pandora README
-----------------------------------------

Since 1.20 the Recovery Flasher also comes build as an .ELF binary to 
be used on a Pandora stick.

*** This is for Pandora Sticks with DDCv5 or earlier!
    (prefferabely DDCv4, that's what used before v6 ;-) )
    On DDCv6 this will NOT work, but on there you can use the EBOOT
    version, it works fine from DDCv6.

Since 1.50 there's also a "ressurection.prx" replacement for DDCv7.
With that, when booting a DDCv7 magicstick you'll get Recovery Flasher
instantly, instead of the original DDC main menu.
Though, if needed, you can jump into the original DDC menu from within
Recovery Flasher to use it's functions.


This way you can use the CFW flashing functions of this tool from a 
Pandora boot as well, to "unchain" or unbrick or whatever.

The backup/restore functions are NOT available in "Pandora Mode".
The app launching function may not work for all apps as well.


Installation - DDCv4/5 or earlier
========================================

There are two ways of installing this - but both require a set up and 
working DDC magicstick:

The preffered way is to launch the RFLASH.ELF via jas0nuk's ELF-Menu 
(ELF-Menu itself can be easiely installed with "Pandora Installer").
Copy the RFLASH.ELF to /elf on your magicstick.
Also copy the original Recovery Flasher's EBOOT.PBP there! *)
Last but not least, copy all (propperly renamed) Sony updaters of the 
FWs you plan to install there as well (i.e. 371.PBP, 401.PBP).

*) About the EBOOT.PBP required for the RFLASH.ELF:
If you also have the normal version located in /PSP/GAME/RECOVERY you
don't need to copy the EBOOT into the /elf folder as well.
If the EBOOT isn't found in /elf it will be found in
/PSP/GAME/RECOVERY.
If you don't have the RECOVERY folder on the stick but still you
don't want have an additional EBOOT.PBP in the /elf folder (due
to it showing up in ELF-Menu or such) you can RENAME the EBOOT.PBP
in the /elf folder to RFLASH.DAT - and it will be found as well but
no longer show up in ELF-Menu.


The other way would be to use it as a ressurection.elf replacement.
Installation is pretty much the same as above, only that you copy 
everything to /kd instead and you rename the RFLASH.ELF to 
ressurection.elf before copying it over.


Installation - DDCv7
========================================

!!!  THIS *ONLY* WORKS ON DDCv7, NOT ON THE v6 OF DDC  !!!

To use Recovery Flasher as a replacement for the original DDC
mainmenu, perform the following steps:

- open up your memstick and browse to the "/TM/DC7/kd" folder

- locate the file "resurrection.prx"

- rename "resurrection.prx" to "ddc.prx"

- now copy the "resurrection.prx" from the "Pandora" folder of the
Recovery Flasher archive into the /TM/DC7/kd folder.

- make sure the original Recovery Flasher is propperly installed
in /PSP/GAME/RECOVERY
(this is important, as additionall files will be loaded from the
EBOOT.PBP)


Done.
When now booting up your DDCv7 stick you will get Recovery Flasher
instead of the original DDC menu.

If you want to use the original DDC, simply choose "Go to original
DDC menu" in the exit option available in DDC mode.
