# Mario Kart 8 Studio
As the grandiose name of this tool suggests, Mario Kart 8 Studio is a project I started with the intent of making an editor for the game Mario Kart 8. My goal was to create a usable editor which works across all platforms, and is completely open source.

Although that goal never completely came to fruition, this project still has a special place in my heart because was really my first foray into writing a useful program. Building a tool like this has given me much direction into where I can grow my skills. In addition to the learning experience, I take pride in what I do have to show here. MK8Studio supports opening decompressed BFRES files, exporting, and injecting textures, for a very limited set of texture formats. It's a little janky, but I'm happy that it has gotten this far.

The most interesting part of the codebase to me is the [annotated image deswizzling code](Source/Formats/Textures/GX2ImageBase.cpp) that was adapted from [addrlib](https://github.com/decaf-emu/addrlib).

If you are looking for a more functional editor, see [Switch-Toolbox](https://github.com/KillzXGaming/Switch-Toolbox).

Main UI:

![](Screenshots/MainUI.png)

Exporting a Texture:

![](Screenshots/Export.png)

Injecting a Texture:

![](Screenshots/Inject.png)

## Build
MK8Studio can be built from the command line using `qmake`. For example, on *nix:
```
mkdir Build
cd Build
qmake ../Source
make
```
Alternatively, you can open up `Source/MK8Studio.pro` in Qt Creator and build it using that.

## Thanks
I give my thanks to the wonderful Dolphin developers, whose project I have referenced countless times for a reference for good design and practices, the Decaf and Mesa teams for ADDRLib, StapleButter for some of the IO code used, Exzap for helping me with decoding GX2 textures, and AboodXD for creating GTX Extractor and helping me with decoding GX2 textures
