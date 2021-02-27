# XUiDesigner
Very early draft of a X11 LV2 GUI design tool

![XUiDesigner](https://i.imgur.com/wKA2eqO.gif)

## Goal
Provide a easy to use GUI generator tool to create X11 based UI's for LV2 plugins.
Currently only libxputty is supported, but the generated GUI C file could be used probably with other 
widget tool-kits as well, just a wrapper file is needed to translate the generated file to the needs of a toolkit.

## Currently state
XUiDesigner parse the ttl file from a selected plugin and generate the needed controller widgets.
Supported been the usual lv2:port parameter and as well the new atom based LV2_PATCH__writable and LV2_PATCH__readable
so as LV2_ATOM__Path.
XUiDesigner use the environment variable LV2_PATH to scan for plugins when no path is given with the
-p command-line parameter.
Additional control widgets could be created and moved freely around in the top Window, or,
a grid could be displayed and widgets could snap to grid (left, right, Center) to order them easily. 
Control widgets could be grouped in a frame and then the complete group could be moved to the final position in the Window.
Any control widget could be replaced with a other control widget, so for example a Toggle Button could be replaced with a ComboBox,
or a Knob could be replaced with a slider, or . . 

The created GUI could be saved (will be saved under ./Bundle/save.lv2/plugname_ui/)
The saved bundle contain a resource folder with the used images and a makefile which allow to build the plug-in-GUI-library.

XUIDesigner have a test-mode as well, which will build and run the created GUI, and give some useful information out in the terminal.
So you could control if the GUI controller act on the correct port index, have the correct range, and so on.

## Currently supported widget types

 - Knob          -> support horizontal framed png
 - HSlider
 - VSlider
 - Button        -> support png/horizontal framed png
 - Toggle Button -> support horizontal framed png
 - ComboBox
 - Value Display
 - Label
 - VMeter
 - HMeter
 - Frame
 - File Chooser Button
