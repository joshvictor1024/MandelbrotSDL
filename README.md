# MandelbrotSDL

A mandelbrot set fractal explorer using SDL2.0 for rendering

## Features

- Drag with mouse to move
- Scroll to zoom, centered at mouse position

### TODO

- Multithread
- Intrinsics (inspired by olc, the OneLoneCoder Youtube channel)
- Higher resolution than 64-bit float
- Chunk based rendering
- Variable iteration depth
  - Growing color range
- Variable resolution
- Improve code structure
  - Style decision
  - Organize into classes

## DevLog

### 2020-07-04

- First chunk based rendering
  - Single map
  - Not reusing already calculated data yet

### 2020-07-02

- Separate computing iterations from drawing
- SDL wrapped into manager class
- Origin is now topleft, rather than center

### 2020-06-30

- First publish