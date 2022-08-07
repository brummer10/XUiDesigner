# XUiDesigner
A WYSIWYG LV2 X11UI GUI creator tool

![XUiDesigner](https://i.imgur.com/wKA2eqO.gif)

## Goal
Provide a easy to use GUI generator tool to create X11 UI's for LV2 plugins.
Currently only libxputty is supported, but the generated GUI C file could be used probably with other 
widget tool-kits as well, just a wrapper file is needed to translate the generated file to the needs of a toolkit.

## Currently state
XUiDesigner parse the ttl file from a selected plugin and generate the needed controller widgets.
Supported been the usual lv2:port parameter and as well the new atom based LV2_PATCH__writable and LV2_PATCH__readable
so as LV2_ATOM__Path.
XUiDesigner use the environment variable LV2_PATH to scan for plugins when no path is given with the
-p command-line parameter.
So you could easily create a GUI for a existing plugin.
A integrated Color chooser allow to create a Color theme for your GUI.
The created GUI could be saved as UI-Bundle, which then could be build (just make) and installed (just make install)
to replace or provide a new GUI for the plugin. 

You could as well create a LV2 plugin from scratch and save it as Full Plugin-Bundle to a selected location.
The project settings window allow to setup the specs (like Author name, URI, Audio/Midi ports, etc.) for your plugin 
XUIDesigner save the bundle in a git repository format, contain a working LV2 plugin with all needed resources 
(ttl files, converted C files from used images, etc.) and build files to build, install and run the new generated plugin.
All you need to do to finish your plug is to implement your DSP part.

XUiDesigner could now parse faust (*.dsp) files and generate a full LV2 Bundle for them. 
A faust (*.dsp) file could be given by the command-line parameter -f or simply by drag it on the Designer window. 
It will create all control widgets which you could then rework to your needs before you save your Bundle.

Control widgets could be created and moved/resized freely around in the top Window.
A grid could be displayed and widgets could snap to grid (left, right, Center) to order them easily. 
Control widgets could be grouped in a frame or a tab box and then the complete group could be moved to the final position in the Window.
Any control widget could be replaced with a other control widget, so for example a Toggle Button could be replaced with a ComboBox,
or a Knob could be replaced with a slider, or . . 
You could set the range for a controller, and it's default value, You could create enums for a ComboBox, . .

Most Control widgets could be replaced with images you could select from a included file browser.

XUIDesigner have a test-mode as well, which will build and run the created GUI, and give some useful information out in the terminal.

## Workflow
Here is a short introducion 
[Wiki](https://github.com/brummer10/XUiDesigner/wiki/XUiDesigner)

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
 - TabBox
 - WaveView
 - File Chooser Button

## Build

- git submodule init
- git submodule update
- make
- sudo make install # will install into /usr/bin
