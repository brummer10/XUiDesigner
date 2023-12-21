
## build for MSWindows cross compile on Linux

To build a generated LV2 plug cross compiled on Linux for MSWindows the easiest way is to use 

[PawPaw](https://github.com/DISTRHO/PawPaw) 

to set up the needed dependencies to build for MSWindows.

The advance of it is that PawPaw builds the dependencies as static libraries

so that the resulting LV2 plug comes without any external dependencies.

To do so, install wine and the mingw g++ and gcc compiler packages (w64-x86-64, w64-i686)

and clone the repository 

`git clone https://github.com/DISTRHO/PawPaw`

build the windows build environment.

`cd PawPaw`

`./bootstrap-plugins.sh win64`

That may take a while, . . .

You need to build the environment only once.

To use the build environment run

`source local.env win64`

from the PawPaw root directory.

That will export all needed settings to build

MSWindows compatible binaries within this terminal session.

After source the local.env you could cd to your plug and run

`make`

to build your binaries for windows.

# using build.sh

For using with [PawPaw](https://github.com/DISTRHO/PawPaw). When you've cloned [PawPaw](https://github.com/DISTRHO/PawPaw) 

and have created the windows (win64) build environment you may as well create a

build environment for Linux. The advance of it is that [PawPaw](https://github.com/DISTRHO/PawPaw) 

creates static libraries to build your plug, that means in turn, less external dependency's

for the final application/plugin. To do so, run

`./bootstrap-plugins.sh linux`

from the [PawPaw](https://github.com/DISTRHO/PawPaw) directory

When done you could simply use build.sh like so

`./libxputty/build.sh windows`

`./libxputty/build.sh linux`

build.sh will create a sub shell, load the selected build environment and run make in it. 

When done, the sub shell will close and there are no external variables loaded

into your main shell. 

So you could use your terminal with it's normal environment setup afterwards.
