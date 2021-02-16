/*
    BitmapSprite Class for use with SmartMatrix Library.

    Sprites are 2D images with transparency that can be rendered at any position on the display.
    Image data is loaded from BMP files on the SD Card.
*/

#ifndef BitmapSprite_h
#define BitmapSprite_h

#include <Arduino.h>
#include <MatrixCommon.h> // from SmartMatrix library

#include <memory>

class BitmapSprite {
    public:
        static void setDisplaySize(uint16_t displayWidth, uint16_t displayHeight);

        int x = 0;
        int y = 0;
        uint8_t alpha = 255;

        BitmapSprite();
        BitmapSprite(const char* filename);
        BitmapSprite(const char* filename, void* destination, size_t allocatedSize);

        bool render(rgb24* buffer);
        uint16_t width() { return wd; };
        uint16_t height() { return abs(ht); };

    private:
        static uint16_t matrixWidth;
        static uint16_t matrixHeight;

        uint16_t wd = 0;
        int16_t ht = 0;
        uint fsize;
        std::shared_ptr<uint8_t> bmpfile;
        uint8_t* image = nullptr;

        void composite(rgb24* bufPtr, int pixel);
        void loadBitmap(const char* filename);
        void loadBitmap(const char* filename, void* destination, size_t allocatedSize);
        void parseHeader(uint8_t* ptr);
        uint32_t read32(uint8_t* ptr);
        uint16_t read16(uint8_t* ptr);
};

#endif
