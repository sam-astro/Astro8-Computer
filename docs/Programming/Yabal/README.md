---
has_children : true
nav_order : 3
layout : default
title : Yabal
parent: Programming
category: Programming
---

# Yabal

Yabal is a high level language which provides a simpler programming interface for the Astro-8 while still being optimized and as capable as Armstrong. [@GerardSmit](https://github.com/GerardSmit) is the sole developer and owner of Yabal.

<br>

## Documentation

The documentation is hosted by the developer here: [https://yabal.dev/docs](https://yabal.dev/docs)

<br>

## In order to use Yabal with the Astro8 emulator, follow these steps:

1. Move the Yabal compiler (`yabal.exe`) to the directory of your code, or [add it to your system PATH environment variable](https://www.architectryan.com/2018/03/17/add-to-the-path-on-windows-10/).
2. Compile your Yabal program. Use the command `yabal.exe build ./yourcode.yabal`. This will create a file called `yourcode.asm` in the same directory. *(Notice how the new file removed the `.yabal` ending, and replaced it with `.asm`)*
3. You can now run this assembly code using the Astro8 emulator like this: `astro8 ./yourcode.asm`
4. As a single line, you can run: `yabal.exe build ./yourcode.yabal; astro8 ./yourcode.asm`.


<!----------------------------------------------------------------------------->

[Commands]: Commands


<!---------------------------------[ Buttons ]--------------------------------->

[Button Commands]: https://img.shields.io/badge/Commands-0288D1?style=flat-square&logoColor=white&logo=Betfair
