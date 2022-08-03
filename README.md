---
permalink : /
nav_order : 1
layout : default
title : Home
---

<div align = center>


<img src="https://github.com/sam-astro/Astro8-Computer/blob/Documentation/images/Astro8-Docs-Logo-Small.png?raw=true"/>


[![Badge License]][License]   [![Button Discord]][Discord Server]


<i>Welcome, you can find the topics in the</i> <br>
<i>left sidebar or search for them above.</i>

</div>


<br>

The Astro-8 is a 16-bit computer design. It has a working **[Logisim Evolution]** circuit, along with an emulator on Windows, Mac, and Linux, for running code at full speed. There is an assembly language, along with a higher level language called Armstrong, plus more which are actively being built by the community.

I created a video about the creation of this and how it works, which you can view here:

[![Button Video]][Video]

## Demos
There are some demos for the Astro-8, such as the pong game and a typing program. You can locate those in the folder `/example_armstrong_programs/`.

<img src="https://raw.githubusercontent.com/sam-astro/Astro8-Computer/main/images/pong.gif"/>


## Programming
The two most supported ways to program for the computer is using the [assembly language](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Instruction%20Set.html) or [Armstrong](https://sam-astro.github.io/Astro8-Computer/docs/Armstrong/README.html).

## Documentation
[![Button Documentation]][Documentation] 

## Installation
### Windows
1. Navigate to [the most recent release](https://github.com/sam-astro/Astro8-Computer/releases), and download the **Windows** version
2. Unzip the downloaded file
### Linux
1. Make sure you have [SDL2 installed](https://wiki.libsdl.org/Installation#supported_platforms) on your system
2. Navigate to [the most recent release](https://github.com/sam-astro/Astro8-Computer/releases), and download the **Linux** version
3. Unzip the downloaded file
### From Source
1. Clone this repository in a command line using `git clone https://github.com/sam-astro/Astro8-Computer.git` OR by downloading the repository as a .ZIP file and unzipping it to your location of choice
2. Make sure you have [SDL2 installed](https://wiki.libsdl.org/Installation#supported_platforms) on your system
3. Enter the directory `Astro8-Computer/Astro8-Emulator/linux-build`
4. Run CMake using `cmake ..` to generate Unix Makefile
5. Run `make -j5` to generate executable
6. The executable is `Astro8-Computer/Astro8-Emulator/linux-build/Astro8-Emulator`

<br>
<br>


## Features

- Fully functional design in **[Logisim Evolution]**

- Created an emulator for running programs at full speed.

<br>


<!----------------------------------------------------------------------------->

[Logisim Evolution]: https://github.com/logisim-evolution/logisim-evolution
[Documentation]: https://sam-astro.github.io/Astro8-Computer/
[Video]: https://www.youtube.com/watch?v=Zt0JfmV7CyI

[License]: LICENSE
[Discord Server]: https://discord.gg/9p82dTEdkN


<!----------------------------------[ Badges ]--------------------------------->

[Badge License]: https://img.shields.io/github/license/sam-astro/Astro8-Computer

<!---------------------------------[ Buttons ]--------------------------------->

[Button Documentation]: https://img.shields.io/badge/Documentation-008FC7?style=flat-square&logoColor=white&logo=GitBook
[Button Video]: https://img.shields.io/badge/Video-c91111?style=flat-square&logoColor=white&logo=YouTube
[Button Discord]: https://img.shields.io/badge/Discord_Server-573f75.svg?style=social&logo=Discord
