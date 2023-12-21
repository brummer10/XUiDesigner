# XUiDesigner
A WYSIWYG LV2 X11UI GUI creator tool

![XUiDesigner](https://i.imgur.com/wKA2eqO.gif)

## Goal
Provide a easy to use GUI generator tool to create UIs for LV2 plugins.

## Currently state
XUiDesigner parse the ttl file from a selected plugin and generate the needed controller widgets.
Supported been the usual lv2:port parameter and as well the new atom based LV2_PATCH__writable and LV2_PATCH__readable
so as LV2_ATOM__Path.
XUiDesigner use the environment variable LV2_PATH to scan for plugins when no path is given with the
-p command-line parameter.
So you could easily create a GUI for a existing plugin.
A integrated Color chooser allow to create a Color theme for your GUI.
Each Widget could be colored individual, or, when selected, a color theme could be used for all widgets.
The created GUI could be saved as UI-Bundle, which then could be build (just make) and installed (just make install)
to replace a exsiting one or provide a new GUI for the LV2 plugin.
For later rework the UI, a json file is added, which you could load per drag 'n drop into XUiDesigner.

You could as well create a LV2 plugin from scratch and save it as Full Plugin-Bundle to a selected location.
The project settings window allow to setup the specs (like Author name, URI, Audio/Midi ports, etc.) for your plugin 
XUIDesigner save the bundle in a git repository format, contain a working LV2 plugin with all needed resources 
(ttl files, converted C files from used images, etc.) and build files to build, install and run the newly generated plugin.
All you need to do to finish your plug is to implement your DSP part.

XUiDesigner could also parse faust (*.dsp) files and generate a full LV2 Bundle for them (including MIDI support). 
A faust (*.dsp) file could be given by the command-line parameter -f or simply by drag it on the Designer window. 
It will create all control widgets which you could then rework to your needs before you save your Bundle.
If you wish you could add a Virtual MIDI Keyboard to the UI for a faust synth.

When going to save your work, the best choice is pre-selected in the format selector, change that only when you know what you do.
XUiDesigner save the generated source code as git repository to make it easy to share your work.
The source code could be used to build LV2 plugs capable to run on Linux or Windows based machines.
MacOS is currently not supported. 

Control widgets could be created and moved/resized freely around in the top Window.
A grid could be displayed and widgets could snap to grid (left, right, Center) to order them easily. 
Control widgets could be grouped in a frame or a tab box and then the complete group could be moved to the final position in the Window.
Any control widget could be replaced with a other control widget, so for example a Toggle Button could be replaced with a ComboBox,
or a Knob could be replaced with a slider, or . . 
You could set the range for a controller, and its default value, you could create enums for a ComboBox, . .

Most Control widgets could be replaced with images you could select from a included file browser.

XUIDesigner has a test-mode as well, which will build and run the created GUI, and give some useful information out in the terminal.

XUiDesigner saved the UI settings also into a json file which could be used to rework the UI at any time later.
This could be loaded by drag 'n drop into the designer interface.

## Workflow
Here is a short introduction 
[Wiki](https://github.com/brummer10/XUiDesigner/wiki/XUiDesigner)

## Currently supported widget types

 - Knob          -> support horizontal framed png
 - HSlider       -> support horizontal framed png
 - VSlider       -> support horizontal framed png
 - Button        -> support single png/horizontal framed png
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
 - Virtual MIDI Keyboard 

## Build

- git submodule init
- git submodule update
- make
- sudo make install # will install into /usr/bin
