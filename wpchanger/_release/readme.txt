HELLCAT's Wallpaper-Changer
==============================
(if you find spelling mistakes or other typos in here, bare with me, this was written at 5am in the morning, with NO coffee....)

Feedback, comments, suggestiones can be put here:
http://forums.qj.net/showthread.php?t=132296


This tiny, little something was inspired by a thread at the QJ forums, where someone asked for such a thing, but noone could find one :-D
So I though "Hey, can't hurt to just make a quick one" :-)

And so, here we are:

The name's the deal: This small CFW plugin changes your XMB background, a.k.a. "paper of the wall", or in short "wallpaper".
That's it. Not more, not less.
(I tested it myself on a PSP Slim running 3.60-M33 and 3.90-M33-2 and a Fat PSP running 3.90-M33)

However, you have a few options:

- Two "modes": "Everytime" and "Once per day" (see below)
- Predefined images for any specific date
- change the XMB background *BEHIND* the waves (a.k.a. 01-12.bmp) - WITHOUT writing to flash! ;)
- Randomizing/Changing of PTF themes
- Randomizing/Changing of the Gameboot

IMPORTANT:
Only (!) put 480x272 .BMP files into the wallpaper folders!
Using differently sized ones or even other file formats (like .JPG, .GIF, .PNG or the such) will cause weird'n'strange side effects. (all will be fine once a 480x272 .BMP is used again, or the wallpaper is disabled completely)
Don't do that.

An even more strict execption are the "wavepapers" (see bellow), those sizes are quite smaller!

N00b-Note:
You must have set "Use Wallpaper" to "ON" or "YES", or whatever the phrase is, in the Theme-Settings of your PSP for the papers to show up! ;-)


Installation
===============

Copy wpchanger.prx into /seplugins on your memstick (create the folder, if it doesn't exist).
Open (or create) vsh.txt and add this line:
ms0:/seplugins/wpchanger.prx

Done, base installation is finished. Didn't hurt, eh? ;-)

Now go into recovery menu and enable the plugin.


Usage
========

This wallpaper changer fetches the images to use from the folder /PICTURE/WALLPAPER on your memstick.
From all images you put in there, this wallpaper changer will pick one randomly and set it as the next wallpaper for the XMB.

Wallpapers BEHIND the XMB waves
----------------------------------
The wallpaper changer can also change/randomize the background behind the waves in the XMB, so you keep the waves showing on top.
The nice thing: It does it *WITHOUT TOUCHING THE FLASH0:!* - so this is 100% safe! :)
(if you want prove: remove memstick and reboot, you'll have the original background back.

The wallpapers for the background behind the waves got into the following folder on your memstick:
/PICTURE/WAVEPAPER

They usually have a resolution of 60x34 - see note 1.

** NOTE 1: **
Usual restrictions for 01-12.bmp (a.k.a. background behind waves) replacement still apply (filesizes and such)!

** NOTE 2: **
If you only get a white background, you don't have a propper multi-image 01-12.bmp (or 13-27.bmp) replacement!
There are two things you can do about this:
1) Select the very first of the Slims additional colors (that pinky one) or the silver/grey color for January (works on Fat+Slim).
2) with this plugin came two PC based tools, use the "Wavepaper Stitcher" to make propper (99.9% compatible) 01-12.bmp (or 13-27.bmp) replacement multi-image .BMP
Find more details in the README in the PC_Tools folder of this archive :-)

Date-Based wallpapers
------------------------
Like stated, you can set a predefined image for any given date.
The special (fixed) wallpaper for a specific day are fetched from /PICTURE/DATEPAPER
The names of the images in there follow a very simply rule: mm-dd.bmp with "mm" beeing the month (two digits, WITH leading zeros!) and "dd" is the day of the month (again, with leading zeros).
So, if you want to make a special wallpaper for January 12th, you'd name the file 01-12.bmp and put it into /PICTURE/DATEPAPER so the complete path would be
/PICTURE/DATEPAPER/01-12.bmp

As another example, for March, 33rd ;-) it'd be:
/PICTURE/DATEPAPER/03-33.bmp

If a special (fixed) wallpaper is defined this way for the current day, only this wallpaper will be set instead of a random one from the .../WALLPAPER directory.
SO you will see the special wallpaper all day long (unless you delete it from .../DATEPAPER, which will result in a random wallpaper again).

Obviously, you can mix both kinds of wallpapers.
Usually having random ones and for special days (birthday, aniversary, reminder, whatever....) you can set a predefined and fixed one.

Modes ("Evertime" and "Once per day")
----------------------------------------
By default a new wallpaper is set on EVERY XMB startup. That also counts in leaving a game or homebrew!
If you only want to have your wallpaper changed once a day, you can "tell" the plugin to do so:
When turning on your PSP, hold the /\ (Triangle) button pressed until the XMB shows up.
This will switch the wallpaper changer into "Once per day" mode and the wallpaper will only be changed on the first startup of a day.

If you feel more randomish again, holding O (Circle) while turning on the PSP switches back to "Everytime" mode.

PTF theme randomizing
------------------------
OK, this is the real easy part ;-)
If you use PTF themes, you know that the PSP uses the folder /PSP/THEME for it, if you choose to change the theme the PSP it gives you the PTFs in there to choose from.
The Wallpaper-Changer also just uses this folder. If it existst, and has .PTFs in it, it grab one randomly and sets it as current theme.
Short and simple :)

You can disable the changing/randomizing of .PTF themes, for the case you have some of them stored on your memstick but just don't want your current theme changed by the Wallpaper-Changer.
To do so, hold [] (Square) while booting the PSP until the XMB shows up - this will disable the changing/randomizing of .PTF themes.
To turn it back on, hold [] again during boot and it'll toggle back to changing the themes.

Gameboot randomizing
-----------------------
This one is really easy as well....
Put your collection of gameboots in /PSP/GAMEBOOT/ on your memstick and a random one of them will be played when you launch a game/app.

Custom XMB wave randomizing
------------------------------
Same as gameboot, just place your system_plugin_bg.rco replacements in
/PSP/WAVE/ on your memstick

*NOTE*
A custom XMB wave may/will cause the gameboot to not work propperly!
This is an effect caused by the custom wave hack and NOT related to the operation of this plugin, nor can it be fixed by it!

XMB font randomizing
-----------------------
Again, same as gameboot, only for the fonts you want to use this folder on your memstick:
/PSP/FONT/

*WARNING*
Placing an impropper ltn0.pgf replacement (i.e. a propper font file) in there WILL cause the PSP to display the BSOD on boot!
If you get the BSOD when booting up, just delete any non propper font file and everything will be fine again!




Well, this is pretty much it.

Have fun! :-)
This little goodie is open-source, if you actually look at and maybe use the sourcecode of this, please see LICENSE.TXT for more details.

---==> HELLCAT <==---
