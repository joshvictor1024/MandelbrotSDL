# MandelbrotSDL

Simple library for generating persistent mandelbrot fractal. Reduces computation by storing results, which lowers CPU usage but increases memory usage.

## Features

- Drag with mouse to move
- or use arrow keys to move
- Use Z/X to zoom, centered at mouse position
- Set `Graphics` parameters in 'constants.h' to set initial view

## Requirements

- Relies on GPU texture for image resizing, uses SDL2
- Uses multiple threads

## Release Notes

### v0.2.0

- Improved performance using multithread

### v0.1.0

- First publish