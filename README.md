# pxshop
-------------------------------------------------------------------
------------This is PxShop - a command line image editor-----------
-------------------------------------------------------------------
FEATURES:
-Open and create indexed color bitmaps
-Edit images with a selection of brush modes
-Import, export and edit color palettes
-Optionally writes a configuration file for choice default settings
-------------------------------------------------------------------
SUPPORT:
Supported terminal environments:
-xterm-256color
-rxvt-unicode-256color
-Should work on any terminal which supports 256 re-definable colors

Supported OS:
-Tested on Ubuntu 22.04.3
-Made with unix usage in mind

Image files:
-Bitmaps V3.0+
-No current compression support for RLE encoded files
-Bitrates 1, 2, 4 and 8 Supported
-supports up to 250 colors
-------------------------------------------------------------------
DOCUMENTATION:
Launching:
./pxshop open-file.bmp              // To open an image
./pxshop new-file.bmp width height  // To create a new image

Operation:
The main screen shows the current image, as well as a selector
wheel at the top of the screen which shows your currently selected
painting color, along with a selection of other palette colors that
can be swapped to.
Controls:
arrow keys - move cursor
Q/E - change selected color
W - select currently hovered over color
R - lock cursor (pan canvas)
T - recenter camera
space - paint
P - flood fill painting mode
O - line drawing mode
^C - view these controls on-screen
^S - save the current image to disc
^L - exit the program
-------------------------------------------------------------------
^Z - settings menu:
This menu lets you configure session settings such as the bitrate
to write the file in, the UI color scheme, autosaving, and the
default palette to use for new files. The current color palette can
also be exported to disc as a .hex file, or can be overwritten with
an imported palette.
Controls:
arrow keys - change selection
space - select
^W - write default settings to disc
^Z - close menu
-------------------------------------------------------------------
^X - palette menu
This menu lets you view the currently loaded color palette. You can
add and delete palette entries, swap colors, edit colors and modify
the color wheel from here.
Controls:
arrow keys - change selection
Q/E - change color wheel selection
A - add new color entry
S - add selected color to color wheel
D - delete selected color entry
F - edit selected color's RGB values
G - swap two color entries
^X - close menu
-------------------------------------------------------------------
Included are a selection of example images made/edited with PxShop,
as well as several community made color palettes to start off with.
Credit for all images/palettes is given in the attached text files.
This is provided as free use software, however if you modify and
redistribute PxShop I'd appreciate credit given for its creation.
-------------------------------------------------------------------
