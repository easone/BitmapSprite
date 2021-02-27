# BitmapSprite
BitmapSprite class for use with SmartMatrix Library.

This class implements fast BMP decoding and rendering code for SmartMatrix displays. Bitmap files are loaded from SD card, and can then be rendered at any position on the display.

Bitmaps are supported in a wide range of formats: 1-bit monochrome, 4-bit and 8-bit indexed, 16-bit color (R5G6R5 and A1R5G5R5), 24 bit RGB, and 32 bit ARGB. These formats can be exported from Gimp or Photoshop. Run-Length Encoding, embedded JPGs and PNGs, and other obscure BMP features are not supported.

The rendering code performs alpha blending using alpha channel information (if present) in combination with an overall sprite transparency alpha that can be used to fade the sprite in and out.

Currently, it's only compatible with Teensy 4.1, but Teensy 3.6 support is planned.
