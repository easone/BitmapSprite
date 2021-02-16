# BitmapSprite
BitmapSprite class for use with SmartMatrix Library.

This class implements fast BMP decoding and rendering code for SmartMatrix displays. Bitmap files are loaded from SD card, and can then be rendered at any position on the display.

Bitmaps should be encoded using ARGB32 BMP format. This format can be exported from Gimp or Photoshop. In particular, it's an uncompressed format that includes full 24-bit color plus an 8-bit alpha channel. The rendering code performs alpha blending using both the alpha channel as well as an overall sprite alpha that can be used to fade the sprite in and out. Support for other common BMP formats is in progress.

Currently, it's only compatible with Teensy 4.1, but Teensy 3.6 support is planned.
