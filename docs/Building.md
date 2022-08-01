---
nav_order : 5
layout : default
title : Building
---

# Building

*How to build this project from source.*

<br>

## Preparation

*Either **Clone** or **Download** the repository.*

<br>

### Clone

1.  Create a new folder in which you will build the project.

2.  Open a new terminal window with your current <br>
    working directory being that of the folders.
    
3.  Execute the following to start cloning:

    ```shell
    git clone https://github.com/sam-astro/Astro8-Computer.git
    ```
    
<br>

### Download

**[Download]** the project and extract <br>
it to a location of your choosing.

<br>
<br>

## Steps

*Starting the build procedure.*

<br>

1.  Navigate to the following directory

    ```shell
    Astro8-Computer/Astro8-Emulator/linux-build
    ```
    
    <br>
    
2.  Generate a Unix Makefile with **CMake**

    ```shell
    cmake ..
    ```
    
    <br>
    
3.  Generate the executable

    ```shell
    make -j5
    ```
    
    <br>
    
4.  Check out the built executable at

    ```
    Astro8-Computer/Astro8-Emulator/linux-build/Astro8-Emulator
    ```

<br>


<!----------------------------------------------------------------------------->

[Download]: https://github.com/sam-astro/Astro8-Computer/archive/refs/heads/main.zip
