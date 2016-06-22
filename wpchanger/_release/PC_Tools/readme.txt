Supplement Tools for the PSP Wallpaper-Changer
==================================================

* Wavepaper Stitcher (wp_stitch.exe)
* Wallpaper Converter (wp_convert.exe)

==================================================

Here you find two small tools, that will aid you with the wallpapers or wavepapers (=background behind XMB waves) to use with Wallpaper-Changer on the PSP.

As source images for theese tools, you can use many different image formats (.gif, .jpg, .bmp, ....) in any possible resolution and any color depth.
The tools take care of all converting that needs to be done :-)


*** WAVEPAPER STITCHER ***
**************************

This tool can be used to make (almost) perfect 01-12.bmp replacements for the background behind the XMB waves.
Since recent firmwares the PSP is pretty picky when it comes to this file, you have to take care of quite a few things.
The Wavepaper-Stitcher will take care of it all for you:

- Resolution of each single image must not exceed 60x34
- The images need a bit depth of 24 bit
- The images need to be in .BMP format
- Each single image in this "multi image" .BMP needs to have an exact filesize

If only one of theese criterias is not met, the resulting .BMP will not propperly work as a 01-12.bmp replacement, most likely all images contained in the .BMP that follow up the one not meeting those criteria will simply not work and you'll get a plain white background instead.
Theese limits are made up by Sony, not me! ;-)

As said, this tool takes care of all that with a single mouseclick.

Run the "wp_stich.exe" from this folder to get the Wavepaper-Stitcher main window.
There you will see two buttons, one for loading an image and one for saving the 01-12.bmp replacement.


-- Make a 01-12.bmp replacement with one image --
-------------------------------------------------

This might be what you usually want.
Since the Wallpaper-Changer is randomizing the wavepapers anyway, you would want a wavepaper .BMP that shows the same image for all possible color selections in the PSPs Theme-Settings.
Do do so, click on the button for loading an image and select the one you want.
You will now see an original size preview of the resulting image.
Click the button for saving, choose a filename and you're already done.
Eihter copy it, or save it directly, into the /PICTURE/WAVEPAPER folder on the memstick and enjoy the result :-)


-- Make a 01-12.bmp replacement with multiple images --
-------------------------------------------------------

If you want a wavepaper that shows a different image for every possible color selection in the PSPs Theme-Settings you can do so as well.
Basically it's very much the same as making it with only one image.
Collect all the images you want to use, put them into one folder and name them 01.bmp, 02.bmp, 03.bmp and so forth.
For a 01-12.bmp (Fat and Slim) replacement wavepaper you want to make it up to 12.bmp - obvious, eh? ;-) For a 13-27.bmp (the Slims new additional colors) make them up to 15.bmp.
From there on proceed exactely as when using only one image, but select the 01.bmp for loading.
The tool will then automatically load the other images one after another and put them all into the new wavepaper file.



*** WALLPAPER CONVERTER ***
***************************

This one's even more easy.
It only has one button and a large preview area.
Click the button and select any image file you want - that's already it. :-D

You will see a PSP size (480x272) preview and the tool will save a new file under the name of the original one prefixed with "wp_" as 480x272x24 .BMP.
Just put that file in the /PICTURE/WALLPAPER folder to add it to your random paper collection :-)



That's it!
Have fun....

---==> HELLCAT <==---


Theese tools may be copied by everyone to everyone, even without the original Wallpaper-Changer plugin as long as there's no charge taken for it and this readme is included!
