# libmandelbrot

Simple library for generating persistent mandelbrot fractal. Reduces computation by storing results, which lowers CPU usage but increases memory usage.

## Demo

- Drag with mouse to move
- or use arrow keys to move
- Use Z/X to zoom, centered at mouse position
- Use A/S to change iteration depth

## Requirements

- Relies on GPU texture for image resizing
  - Implemented for SDL2 and above
  - Implemented for olcPixelGameEngine2.0 and above
- Uses multiple threads

## Release Notes

### v0.3.0

- Improved drawing speed using hardware-native pixelformat
- Better graphics abstraction

### v0.2.0

- Improved performance using multithread

### v0.1.0

- First publish