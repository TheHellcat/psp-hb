WHA! You're reading a README? Freaky, boy, freaky ;-)))

k, so, here we go:

P a n d o r a   I n s t a l l e r   f o r   3 . x x +   K e r n e l s
===========================================================================
leave feedback and comments here:
http://forums.qj.net/showthread.php?t=122449

-------------------------
  R e v i s i o n   4
-------------------------
containing, with kind permission of their devs, the additional magic stick
tools: ELF-Menu by jas0nuk, NAND-Tool by cory1492
-------------------------
DDCv6 note:
=============
When you install a DDCv6 using this app, the TimeMachine IPL loader will
automatically installed to the memstick, including a config for it.
When booting this DDCv6 stick with a Pandora battery, the PSP will boot
up normally from the flash (if possible) unless you press one of theese
buttons while powering on:
"Up"    - boots an older, MSIPL.BIN based, Pandora setup
          The MSIPL.BIN will automatically be converted to TM compatible
          format if it is present at DDCv6 installation
"Down" -  Will boot DDCv6
X, [], O or /\ will boot installed TM firmwares
-------------------------


Preface?
----------
Regarding pandorizing of a SLIM battery....

YES, it's possible, it's harmless, it's all cool :)
In the meantime I did it myself, without any problems or trouble.


Required additional files for instant usage:

- The 1.50 update EBOOT renamed as "150.PBP"
- The 3.40 update EBOOT renamed as "340.PBP"
- The 3.80 update EBOOT renamed as "380.PBP"
- The 4.01 update EBOOT renamed as "401.PBP"

All to be in the root of your memstick, and you're ready to fire away!


Middleface?
-------------
I have often been asked for adding functions to backup/restore the
complete EEPROM of the battery, like all other tools do.
Theese functions are not implemented on purpose, and they never will be!

Since the EEPROM of the battery contains way more than just the serial
and most of it is pretty vital for the propper function of the battery
restoring an old or even wrong EEPROM can kill your battery beyond
recovery - many people I know already managed to do so.

However, with revision 3 of this app, there are the options to
backup and restore your battery's serial number to/from a file.
That way you can revert it to the original serial or to any serial
you like by editing the file :-)


Now What?
-----------
The original Pandora-Tools are dependant on 1.50, if you are on 3.71-M33
or the Slim you're out of luck - they don't run.
Even worse, for preparing your MemStick, you need to run Windows based
tools from the commandprompt.
(some may have problems with that)

But fear no more ;-)

This is an installer app for a base Pandora setup.
It runns under *ANY* currently known kernel (that is 1.50 to 4.01) and
gives you many options:

1) Prepare (repartition and format) your memstick for Pandora
   (to reserve space for the MS IPL)
   since revision 3 you can also choose the name after format (see below)
2) Make normal battery to Pandora one
3) Make Pandora battery to normal one
4) Install a full 1.50 to the memstick, including classic Pandora

New since Rev3:
5) Backup current battery serial to file
6) Restore battery serial from file
   (if trying to restore from a backupfile without one beeing present,
    the default serial will simply be used instead)

New since Rev4:
7) Install a DDC MagicStick that runns fine on the Slim and installs
   a more or less recent CFW
   (the classic Pandora from #4 only runns on Fat PSP and installs 1.50)
8) Enhance the DDC stick by installing ELF-Menu and NAND-Tool
   (it all get's installed ready-to-use, no further hazzle)
9) Install the TimeMachine IPL to the MemStick

New since Rev4a:
10) Install a DDCv6 to the magic memstick

The file used for backup/restore the battery serial is:
ms0:/batser.bin

You can edit the contents of this file with a hexeditor to whatever
serial you would like to patch you battery to.

N00B-Note: HEX editor, not text edtior - notepad will not work ;-)

And yes, it does run on 3.71 and even 3.80 - I tested it myself ;-)
(meanwhile I even tested it on 4.01-M33)

Since Revision 2, you also don't need to use the commandprompt
tools for windows anymore!
This installer offers you options to repartition and format
your memstick for Pandora, and to install the MSIPL.BIN file
into the reserved area on the memstick.

If you don't like the default name of the memstick after format
("Pandora") you can override the name by placing a file "msname.txt"
in the root of the memstick before format.
Just put the desired name into the file, don't worry about illegal
characters, anything not allowed for a filename is converted to "_".
Max. length of the name is 11 characters. (you can enter more, but
only the first 11 are read)

You can install any MSIPL.BIN file you want, the original,
or boosters multiloader, for example.
Just name it MSIPL.BIN and use the option from the installer.


How?
------
To use the installer, first you need the 1.50 updater EBOOT named
as "UPDATE.PBP" in the root folder of your memorystick.
As a regular Pandora user you'll have that already, if not copy it!

Now copy the "pan3xx" folder from this archive to
/PSP/GAME/ on your memorystick - and run it!

If you want to install a DDC MagicStick you also need
- 340.PBP   (the 3.40 Updater EBOOT)
- 380.PBP   (the 3.80 Updater EBOOT)
in the root of your MemStick.

For installing DDCv6 you need:
- 401.PBP   (the 4.01 Updater EBOOT)


Done :)


Cool?
-------
Yes!
Since this installer installs a full 1.50, and not only a subset like 
the original installer, you're only one tiny file edit away from
booting a full 1.50 from your memstick to run additional tools or
installers from there before/instead of directly flashing your PSP.

At this point, let me suggest to install the "Extended Pandora Menu" ;-D

To make your magic stick boot a full 1.50 instead of the Pandora menu,
do this:

- Open /kd/pspbtcnf.txt on your memstick
- Find the line "%%/kd/extprxs.elf" and change it to "# %%/kd/extprxs.elf"
(i.e. add the # in front)
- Find the line "# %%/vsh/module/vshmain.prx" and chage it to "%%/vsh/module/vshmain.prx"
(i.e. remove the # in front)

- don't forget - all without the quotes ;-)

- Save and close the file, reboot PSP => full 1.50 from memstick!


Else?
-------
Yah, read the README! Oh wait.... this IS the README....
Cool, in that case, you're done.
Good Bye, have fun ;-)


German?
---------
Auch. F¸r die ganzen mega Hacker im Gulli-Board Kindergarten, die zwar
alle Top-H4x0rs sein wollen, aber kein Wort Englisch kˆnnen, hier
eine Kurzfassung auf Deutsch:

Dieser Installer l‰sst Dich:
1) Den MemStick vorbereiten (umpartitionieren und formatieren) f¸r Pandora
   (wem der Standardname "PANDORA" f¸r den Stick nicht passt, kann seit Rev3
    den Namen auch vor dem Formatieren festlegen - siehe weiter unten)
2) Eine normale Batterie zu Pandora machen
3) Eine Pandora Batterie zu eine normalen machen
4) Eine volle 1.50 - einschlieﬂlich Pandora - auf dem MemStick installieren

Neu seit Rev3:
5) Seriennummer der Batterie in Datei speichern
6) Seriennummer der Batterie aus Datei wiederherstellen
   (wird versucht die Seriennummer wiederherzustellen ohne das eine
    Backupdatei vorhanden ist, wird einfach die standard Seriennummer
    benutzt)

Die Datei die f¸rs Speichern/Laden der Seriennummer benutzt wird ist:
ms0:/batser.bin

Du kannst die Datei auch mit einem HEX(!) Editor ‰ndern um so jede
beliebige Seriennummer f¸r die Batterie zu benutzen.

- und l‰uft auch unter 3.71, 3.80 und auf der Slim!


Um den Namen des MemSticks nach dem umformatieren festzulegen (anstelle
des "PANDORA") einfach eine Datei "msname.txt" ins Hauptverz. des Sticks
speichern, in dem der gew¸nschte Name steht.


Installation:
1.50 Updater EBOOT als "UPDATE.PBP" in's Hauptverzeichniss des MemSticks
kopieren.
Dann das "pan3xx" Verzeichnis dieses Archives nach /PSP/GAME/ kopieren.
STARTEN und los hacken!


Volle 1.50 anstelle des Pandora Men¸s vom Stick starten:

- ÷ffne /kd/pspbtcnf.txt auf dem MemStick
- Finde "%%/kd/extprxs.elf" und ‰ndere zu "# %%/kd/extprxs.elf"
(also ein # am Anfang einf¸gen)
- Finde "# %%/vsh/module/vshmain.prx" und ‰ndere zu "%%/vsh/module/vshmain.prx"
(also das # am Anfang entfernen)

- das alles ohne die Anf¸hrungszeichen!

- Speichern, Schlieﬂen, PSP neustarten => volle 1.50 vom Stick! :-)


Viel Spaﬂ!
