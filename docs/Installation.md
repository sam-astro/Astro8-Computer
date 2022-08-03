---
nav_order : 2
layout : default
title : Installation
---

# Installation

*How to install the emulator.*

<br>

### Windows
1. Navigate to **[the most recent release]**, and download the **Windows** version
2. Unzip the downloaded file
### Linux
1. Make sure you have **[SDL2 installed]** on your system
2. Navigate to **[the most recent release]**, and download the **Linux** version
3. Unzip the downloaded file
### Building From Source
1. Clone this repository in a command line using `git clone https://github.com/sam-astro/Astro8-Computer.git` OR by downloading the repository as a .ZIP file and unzipping it to your location of choice
2. Make sure you have **[SDL2 installed]** on your system
3. Enter the directory `Astro8-Computer/Astro8-Emulator/linux-build`
4. Run CMake using `cmake ..` to generate Unix Makefile
5. Run `make -j5` to generate executable
6. The executable is `Astro8-Computer/Astro8-Emulator/linux-build/Astro8-Emulator`

<br>

*[» Now that the emulator is installed, check out how to get started using it.][Usage]*

<!----------------------------------------------------------------------------->

[SDL2 installed]: https://wiki.libsdl.org/Installation#supported_platforms
[the most recent release]: https://github.com/sam-astro/Astro8-Computer/releases
[Usage]: Usage.md
