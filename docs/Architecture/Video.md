---
layout : default
parent : Architecture
title : Video
category: Architecture
nav_order : 6
---

# Video

There are 2 video modes, Bitmap and Character. In Bitmap mode, you can write individual pixels to the screen. In character mode, you can write numbers, letters, and symbols to the screen. The characters are overlayed on top of the pixels, so they take priority. In this essence, pixels are "the background" while characters are "the foreground"

## Bitmap Mode

In order to write pixels to the screen, it is important to understand the video system of the Astro-8. The screen is `108x108` pixels, meaning there are a total of 11664 memory locations occupied by the screen.

Writing pixels is just like writing to any other part of memory. You simply use whatever command the language provides you with to change that location of memory to a color.

Have a look at the [Memory Layout](https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Memory%20Layout.html) to see where the video memory is mapped.

The colors are stored in 5-bit RGB. Looking at the binary for a single pixel, it would look like this:

```
RRRRR GGGGG BBBBB
```

In order to set each of these values, it is often recommended to use bit-shifting. You can also create a function capable of doing this for you. Here is an example using [Yabal](https://yabal.dev/docs)

```c
inline int get_color(int r, int g, int b) {
    return (r / 8 << 10) + (g / 8 << 5) + (b / 8);
}
```

This function takes 3 input values, and combines them to create a single word


## Characters

The screen supports `18x18` characters, each being `6x6` pixels large
