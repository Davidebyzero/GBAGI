GBAGI: The Game Boy Advance Adventure Game Interpreter
By Brian Provinciano
Revision 2.00
October 3rd, 2004
http://www.bripro.com
===============================================================================
Sierra's original animated adventure games on the Nintendo Game Boy Advance!

 
Here at last, GBAGI, the interpreter/emulator/software that runs your favorite 
original Sierra games including King's Quest 1-4, Space Quest 1-2, Police Quest, 
Leisure Suit Larry, Manhunter 1-2, The Black Cauldron, and more on your Nintendo 
Game Boy Advance.

GBAGI is a Sierra adventure game interpreter/emulator for the Nintendo Game Boy 
Advance. With the use of a flash cart on your GBA, you can play all your AGI 
games on the portable. Not only can you play Sierra's adventure games, but also 
your own, and dozens of homebrew games made by other enthusiasts! 

To use it, simply run the gbinject.exe tool and select the games in which you
wish to put in the ROM. When done, your ROM is ready to be put onto a flash
cart and ready to play!


Change Highlights
=================

-Added a virtual keyboard! Now all keys can be pressed, including CTRL, ALT 
 and SHIFT combinations!
-Now ALL words can be entered in the input dialog by selecting the "More >>" 
 button!
-Added a new improved game select screen
-Added diagonal movement
-Improved audio playback accuracy
-Improved the menu system
-Fixed picture rendering, so now "Space Quest 0: Replicated" is fully playable


Buttons
=======

In Game
=======
Up/Down/Left/Right: Movement
A: Brings up text input dialog ("ENTER/RETURN")
L: Repeat previous text input ("F3")
B: Brings up game menu ("ESC")
Start: Brings up virtual keyboard
R: Repeat previous key pressed on virtual keyboard
Select: Inventory ("TAB")

START+SELECT+A+B: Exit to Game Select Screen

In GUI
======
L: Select previous control
R: Select next control
A: Press button
Up/Down: Select item in listbox, select letter in edit box
Left/Right: Select next/previous set of words in list box



Special Thanks to:
==================

Jeff Frohwein for his "crt0.S v1.26" and "lnkscript" GBA lowlevel base interrupt 
execution code. See Jeff's web site, http://www.devrs.com/gba/

Dovoto for his GBA header files ("keypad.h","screenmode.h","gba.h"). See his
web site, the Nintendo Reverse Engineering Project (http://www.thepernproject.com)

Claudio Matsuoka (http://agi.helllabs.org), for the original AGI executable 
disassembly source (AGI.ASM).



Compiling
=========

The DevKitAdvance GCC compiler is used, but you should be able to use any other
compiler provided you make the correct make files. You must make sure it builds
with the crt0.o in the current directory and not the one in the devkitadv 
directory, or the interrupts will not work.

As it is written in portable ANSI C, I also included a Win32 project file for
C++Builder. It allows you to compile it for Win32 to test it. However, the 
Win32 port does not include sound or proper timing. It could be easily updated
to support this though.



For more information, downloads, and other similar projects, be sure to check
out the official web site: 

http://www.bripro.com

Enjoy!


 
