/*
    BitmapSprite Class for use with SmartMatrix Library.

    Sprites are 2D images with transparency that can be rendered at any position on the display.
    Image data is loaded from BMP files on the SD Card.
*/

#include "BitmapSprite.h"
#include <SD.h>
#include "gammaLUT.h"

uint16_t BitmapSprite::matrixWidth = 0;
uint16_t BitmapSprite::matrixHeight = 0;

void BitmapSprite::setDisplaySize(uint16_t displayWidth, uint16_t displayHeight) {
    matrixWidth = displayWidth;
    matrixHeight = displayHeight;
}


BitmapSprite::BitmapSprite() {
}

BitmapSprite::BitmapSprite(const char* filename) {
    // Reads the SD card and dynamically allocates new memory for image data.
    loadBitmap(filename);
}

BitmapSprite::BitmapSprite(const char* filename, void* destination, size_t allocatedSize) {
    // Reads the SD card and places image data into a statically allocated memory range.
    loadBitmap(filename, destination, allocatedSize);
}

bool BitmapSprite::render(rgb24* buffer) {
    // Renders the sprite to the provided drawing buffer.
    // Returns 0 if the sprite is invisible or not properly initialized.
    if (alpha == 0) return 0; // invisible
    if (!image || wd == 0 || ht == 0) return 0; // not properly initialized
    if (matrixWidth == 0 || matrixHeight == 0) return 0; // display size not set

    if (ht > 0) { // bitmap is stored bottom-to-top

        int topval = y - ht + 1;
        int bottomval = y;
        int leftval = x;
        int rightval = x + wd - 1;

        int startY = max(topval, 0);
        int endY = min(bottomval, matrixHeight - 1);

        int startX = max(leftval, 0);
        int endX = min(rightval, matrixWidth - 1);

        if (startX > endX) return 0;
        if (startY > endY) return 0;

        int pixelRow = (startX - leftval) + (bottomval - startY) * wd;
        int pixel;

        rgb24* bufRowPtr = buffer + startX + startY * matrixWidth;
        rgb24* bufPtr;

        for (int j = startY; j <= endY; j++) {
            bufPtr = bufRowPtr;
            bufRowPtr += matrixWidth;
            pixel = pixelRow;
            pixelRow -= wd;
            for (int i = startX; i <= endX; i++) {
                composite(bufPtr++, pixel++);
            }
        }

    } else { // ht < 0, so bitmap is stored top-to-bottom

        int topval = y;
        int bottomval = y + (-ht) - 1;
        int leftval = x;
        int rightval = x + wd - 1;

        int startY = max(topval, 0);
        int endY = min(bottomval, matrixHeight - 1);

        int startX = max(leftval, 0);
        int endX = min(rightval, matrixWidth - 1);

        if (startX > endX) return 0;
        if (startY > endY) return 0;

        int pixelRow = (startX - leftval) + (startY - topval) * wd;
        int pixel;

        rgb24* bufRowPtr = buffer + startX + startY * matrixWidth;
        rgb24* bufPtr;

        for (int j = startY; j <= endY; j++) {
            bufPtr = bufRowPtr;
            bufRowPtr += matrixWidth;
            pixel = pixelRow;
            pixelRow += wd;
            for (int i = startX; i <= endX; i++) {
                composite(bufPtr++, pixel++);
            }
        }
    }
    return 1;
}

void BitmapSprite::composite(rgb24* bufPtr, int pixel) {
    // helper function performs alpha compositing operation on a single pixel
    // using pixel alpha value times overall sprite alpha
    uint32_t r, g, b, a, rd, gd, bd;
    int byteIndex = pixel * 4;
    b = image[byteIndex++];
    g = image[byteIndex++];
    r = image[byteIndex++];
    a = image[byteIndex];

    if ((a > 0) && (alpha > 0)) {

        a = a * alpha * 257 / 255; // expands 0xFF * 0xFF to 0xFFFF

        if (a < 0xFFFF) {

            r = decodeGamma8to16(r);
            g = decodeGamma8to16(g);
            b = decodeGamma8to16(b);

            rd = decodeGamma8to16((*bufPtr).red);
            gd = decodeGamma8to16((*bufPtr).green);
            bd = decodeGamma8to16((*bufPtr).blue);

            r = encodeGamma16to8((uint16_t)(rd + ((a * (r - rd)) >> 16)));
            g = encodeGamma16to8((uint16_t)(gd + ((a * (g - gd)) >> 16)));
            b = encodeGamma16to8((uint16_t)(bd + ((a * (b - bd)) >> 16)));
        }
        *bufPtr = rgb24(r, g, b);
    }
}

void BitmapSprite::loadBitmap(const char* filename) {
    File file = SD.open(filename);

    if (!file) {
        Serial.println("Error: Could not open file.");
        file.close();
        return;
    }

    fsize = file.size();

    // dynamically allocate array to hold file content
    // use shared_ptr so multiple sprites can point to the same data
    bmpfile.reset(new uint8_t[fsize], std::default_delete<uint8_t[]>());

    if (!bmpfile) {
        Serial.println("Error: Failed to allocate memory.");
        file.close();
        return;
    }

    uint8_t* ptr = bmpfile.get();

    file.read(ptr, fsize);
    file.close();

    // Flush cache just in case...
    if ((uint32_t)ptr >= 0x20200000u) arm_dcache_flush_delete(ptr, fsize);

    parseHeader(ptr);
}

void BitmapSprite::loadBitmap(const char* filename, void* destination, size_t allocatedSize) {
    File file = SD.open(filename);

    if (!file) {
        Serial.println("Error: Could not open file.");
        file.close();
        return;
    }

    fsize = file.size();

    if (fsize > allocatedSize) {
        Serial.println("Error: File too large.");
        file.close();
        return;
    }

    // note: since the memory is statically allocated, do not initialize the shared_ptr.
    bmpfile.reset();

    uint8_t* ptr = (uint8_t*)destination;

    file.read(ptr, fsize);
    file.close();

    // Flush cache just in case...
    if ((uint32_t)ptr >= 0x20200000u) arm_dcache_flush_delete(ptr, fsize);

    parseHeader(ptr);
}


void BitmapSprite::parseHeader(uint8_t* ptr) {

    if (ptr[0] != 'B' || ptr[1] != 'M') {
        Serial.println("Error: Wrong file format (not bitmap).");
        bmpfile.reset();
        return;
    }

    int dataOffset = read32(ptr + 10);
    wd = read32(ptr + 18);
    ht = read32(ptr + 22);
    int bitspp = read16(ptr + 28);

    if (bitspp != 32) {
        Serial.println("Error: Wrong BMP format (ARGB32 expected).");
        bmpfile.reset();
        wd = 0;
        ht = 0;
        return;
    }

    // todo: implement other BMP formats (common Gimp ones)
    // may need to add additional variables to deal with padding, and define bitfield structs
    // make sure compatible with and without colorspace information

    image = ptr + dataOffset;
}

uint32_t BitmapSprite::read32(uint8_t* ptr) {
    // helper function reads 4 bytes into 32-bit little endian int
    // without using un-aligned memory reads (can cause bug on Teensy 3.6)
    return ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
}

uint16_t BitmapSprite::read16(uint8_t* ptr) {
    // helper function reads 2 bytes into 16-bit little endian int
    // without using un-aligned memory reads (can cause bug on Teensy 3.6)
    return ptr[0] | (ptr[1] << 8);
}
