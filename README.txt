How to build

Get source from github
- git clone https://github.com/czchen/TSF-chewing.git

Initial and synchronize submodule
- git submodule init
- git submodule update

Use CMake to generate Visual Studio project file
- cmake -G "Visual Studio 11" <path to TSF-chewing> (Visual Studio 2012 for x86)
- cmake -G "Visual Studio 11 Win64" <path to TSF-chewing> (Visual Studio 2012
  for x64)

Build project with Visual Studio

Copy libchewing/data/*.dat to %WINDIR%/chewing.

Use regsvr32 to register ChewingService.dll (In Debug folder if configure is
Debug).
- regsvr32 ChewingService.dll
