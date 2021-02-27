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
        enum Format { // Supported BMP formats
          RGB1, // 1bpp, indexed
          RGB4, // 4bpp, indexed
          RGB8, // 8bpp, indexed
          XRGB16, // 16bpp, arbitrary bitmask with transparency
          RGB24, // 24bpp, R8G8B8
          ARGB32, // 32bpp, R8G8B8 or A8R8G8B8
          XRGB32 // 32bpp, arbitrary bitmask with transparency
        };
        
        static uint16_t matrixWidth;
        static uint16_t matrixHeight;

        uint16_t wd = 0;
        int16_t ht = 0;
        uint fsize;
        std::shared_ptr<uint8_t> bmpfile;
        uint8_t* image = nullptr;

        BitmapSprite::Format format;
        uint8_t* palette = nullptr;
        bool alphaChannel = false;
        uint16_t rowBytes = 0;
        uint32_t rMask = 0;
        uint8_t rScale = 0;
        uint8_t rShift = 0;
        uint32_t gMask = 0;
        uint8_t gScale = 0;
        uint8_t gShift = 0;
        uint32_t bMask = 0;
        uint8_t bScale = 0;
        uint8_t bShift = 0;
        uint32_t aMask = 0;
        uint8_t aScale = 0; 
        uint8_t aShift = 0;

        void composite(rgb24* bufPtr, uint row, uint col);
        void loadBitmap(const char* filename);
        void loadBitmap(const char* filename, void* destination, size_t allocatedSize);
        void parseHeader(uint8_t* ptr);
        uint32_t read32(uint8_t* ptr);
        uint16_t read16(uint8_t* ptr);
        uint8_t maskToScale(uint32_t mask);
        uint8_t maskToShift(uint32_t mask);
};

#endif
