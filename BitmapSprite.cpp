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

    int topval, bottomval, leftval, rightval;

    if (ht > 0) { // bitmap is stored bottom-to-top
        // place sprite so that bottom left corner is at (x,y) on screen)
        topval = y - abs(ht) + 1;
        bottomval = y;
        leftval = x;
        rightval = x + wd - 1;
    } else { // bitmap is stored top-to-bottom
        // place sprite so that top left corner is at (x,y) on screen)
        topval = y;
        bottomval = y + abs(ht) - 1;
        leftval = x;
        rightval = x + wd - 1;
    }

    int startY = max(topval, 0);
    int endY = min(bottomval, matrixHeight - 1);
    int startX = max(leftval, 0);
    int endX = min(rightval, matrixWidth - 1);

    if (startX > endX) return 0;
    if (startY > endY) return 0;

    rgb24* bufRowPtr = buffer + startX + startY * matrixWidth;
    rgb24* bufPtr;

    for (int j = startY - y; j <= endY - y; j++) {
        bufPtr = bufRowPtr;
        bufRowPtr += matrixWidth;
        for (int i = startX - leftval; i <= endX - leftval; i++) {
            composite(bufPtr++, abs(j), i);
        }
    }
    return 1;
}

void BitmapSprite::composite(rgb24* bufPtr, uint row, uint col) {
    // helper function performs alpha compositing operation on a single pixel
    // using pixel alpha value times overall sprite alpha
    uint32_t r = 0, g = 0, b = 0, a = 255;

    switch (format) {
        case RGB1: { // 1bpp, indexed
                uint8_t* pixPtr = image + row * rowBytes + (col >> 3);
                int bitindex = 7 - (col & 7);
                uint8_t c = (*pixPtr & (1 << bitindex)) >> bitindex;
                uint8_t* palPtr = palette + c * 4;
                b = *palPtr++;
                g = *palPtr++;
                r = *palPtr++;
            } break;
        case RGB4: { // 4bpp, indexed
                uint8_t* pixPtr = image + row * rowBytes + (col >> 1);
                uint8_t c;
                if (col & 1) { // low nibble
                    c = (*pixPtr & 0x0F);
                } else { // high nibble
                    c = (*pixPtr & 0xF0) >> 4;
                }
                uint8_t* palPtr = palette + c * 4;
                b = *palPtr++;
                g = *palPtr++;
                r = *palPtr++;
            } break;
        case RGB8: { // 8bpp, indexed
                uint8_t* pixPtr = image + row * rowBytes + col;
                uint8_t c = *pixPtr;
                uint8_t* palPtr = palette + c * 4;
                b = *palPtr++;
                g = *palPtr++;
                r = *palPtr++;
            } break;
        case XRGB16: { // 16bpp, arbitrary bitmask with transparency
                uint8_t* pixPtr = image + row * rowBytes + col * 2;
                uint32_t pixWord = read16(pixPtr);
                r = ((pixWord & rMask) * rScale) >> rShift;
                g = ((pixWord & gMask) * gScale) >> gShift;
                b = ((pixWord & bMask) * bScale) >> bShift;
                if (alphaChannel) a = ((pixWord & aMask) * aScale) >> aShift;
            } break;
        case RGB24: { // 24bpp, R8G8B8
                uint8_t* pixPtr = image + row * rowBytes + col * 3;
                b = *pixPtr++;
                g = *pixPtr++;
                r = *pixPtr++;
            } break;
        case ARGB32: { // 32bpp, R8G8B8 or A8R8G8B8
                uint8_t* pixPtr = image + row * rowBytes + col * 4;
                b = *pixPtr++;
                g = *pixPtr++;
                r = *pixPtr++;
                if (alphaChannel) a = *pixPtr;
            } break;
        case XRGB32: { // 32bpp, arbitrary bitmask with transparency
                uint8_t* pixPtr = image + row * rowBytes + col * 4;
                uint32_t pixWord = read32(pixPtr);
                r = ((uint64_t)(pixWord & rMask) * rScale) >> rShift;
                g = ((uint64_t)(pixWord & gMask) * gScale) >> gShift;
                b = ((uint64_t)(pixWord & bMask) * bScale) >> bShift;
                if (alphaChannel) a = ((uint64_t)(pixWord & aMask) * aScale) >> aShift;
            } break;
    }

    uint32_t rd = 0, gd = 0, bd = 0;


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

    bool invalidFormat = false;

    if (!(ptr[0] == 'B' && ptr[1] == 'M')) invalidFormat = true;
    size_t dataOffset = read32(ptr + 10);
    size_t headerSize = read32(ptr + 14);
    if (!(headerSize == 40 || headerSize == 52 || headerSize == 56 || headerSize == 108 || headerSize == 124)) invalidFormat = true;
    wd = read32(ptr + 18);
    ht = read32(ptr + 22);
    int planes = read16(ptr + 26);
    if (!(planes == 1)) invalidFormat = true;
    int bitspp = read16(ptr + 28);
    if (!(bitspp == 1 || bitspp == 4 || bitspp == 8 || bitspp == 16 || bitspp == 24 || bitspp == 32)) invalidFormat = true;
    int compression = read32(ptr + 30);
    size_t imageSize = read32(ptr + 34);
    int colorsUsed = read32(ptr + 46);

    if (wd == 0 || ht == 0) {
        invalidFormat = true;
    } else {
        rowBytes = imageSize / abs(ht);
    }

    // check that bitmap data fits within file size
    if (dataOffset + imageSize > fsize) invalidFormat = true;

    switch (bitspp) {
        case 1:
            format = RGB1;
            if (!(compression == 0)) invalidFormat = true; // only uncompressed type supported
            palette = ptr + 14 + headerSize;
            if (colorsUsed == 0) colorsUsed = 2; // 1 << bitspp
            if (14 + headerSize + colorsUsed * 4 > dataOffset) invalidFormat = true;
            break;
        case 4:
            format = RGB4;
            if (!(compression == 0)) invalidFormat = true; // only uncompressed type supported
            palette = ptr + 14 + headerSize;
            if (colorsUsed == 0) colorsUsed = 16; // 1 << bitspp
            if (14 + headerSize + colorsUsed * 4 > dataOffset) invalidFormat = true;
            break;
        case 8:
            format = RGB8;
            if (!(compression == 0)) invalidFormat = true; // only uncompressed type supported
            palette = ptr + 14 + headerSize;
            if (colorsUsed == 0) colorsUsed = 256; // 1 << bitspp
            if (14 + headerSize + colorsUsed * 4 > dataOffset) invalidFormat = true;
            break;
        case 24:
            format = RGB24;
            if (!(compression == 0)) invalidFormat = true; // only uncompressed type supported
            break;
        case 16:
            format = XRGB16;
            if (compression == 0) { // no compression
                // default R5G5B5 format
                rMask = 0b11111 << 10;
                gMask = 0b11111 << 5;
                bMask = 0b11111;
                aMask = 0;
            } else if (compression == 3) { // bitfield compression
                rMask = read32(ptr + 54);
                gMask = read32(ptr + 58);
                bMask = read32(ptr + 62);
                if (headerSize == 40 || headerSize == 52) {
                    aMask = 0;
                } else {
                    aMask = read32(ptr + 66);
                }
            } else { // other compression types not supported
                invalidFormat = true;
            }
            break;
        case 32:
            if (compression == 0) { // no compression
                format = ARGB32; // default R8G8B8 format
                rMask = 0x00FF0000;
                gMask = 0x0000FF00;
                bMask = 0x000000FF;
                aMask = 0;
            } else if (compression == 3) { // bitfield compression
                rMask = read32(ptr + 54);
                gMask = read32(ptr + 58);
                bMask = read32(ptr + 62);
                if (headerSize == 40 || headerSize == 52) {
                    aMask = 0;
                } else {
                    aMask = read32(ptr + 66);
                }
                if (rMask == 0x00FF0000 && gMask == 0x0000FF00 && bMask == 0x000000FF && (aMask == 0 || aMask == 0xFF000000)) {
                    format = ARGB32;
                } else {
                    format = XRGB32;
                }
            } else { // other compression types not supported
                invalidFormat = true;
            }
            break;
        default:
            invalidFormat = true;
    }

    if (!invalidFormat && (bitspp == 16 || bitspp == 32)) {
        // check that color masks are contiguous and non-overlapping
        bool overlappingMasks = (rMask & gMask) || (rMask & bMask) || (rMask & aMask) || (gMask & bMask) || (gMask & aMask) || (bMask & aMask);
        bool rMaskNotContiguous = rMask & (rMask + (1 << __builtin_ctz(rMask)));
        bool gMaskNotContiguous = gMask & (gMask + (1 << __builtin_ctz(gMask)));
        bool bMaskNotContiguous = bMask & (bMask + (1 << __builtin_ctz(bMask)));
        bool aMaskNotContiguous = aMask & (aMask + (1 << __builtin_ctz(aMask)));
        if (overlappingMasks || rMaskNotContiguous || gMaskNotContiguous || bMaskNotContiguous || aMaskNotContiguous) {
            invalidFormat = true;
        } else {
            // Compute scale factors and shifts to scale color information to 8 bit.
            rScale = maskToScale(rMask);
            gScale = maskToScale(gMask);
            bScale = maskToScale(bMask);
            aScale = maskToScale(aMask);
            rShift = maskToShift(rMask);
            gShift = maskToShift(gMask);
            bShift = maskToShift(bMask);
            aShift = maskToShift(aMask);
            alphaChannel = (bool) aMask; // false if no alpha bitmask
        }
    }

    if (invalidFormat) {
        Serial.println("Error: Unsupported file format.");
        bmpfile.reset();
        wd = 0;
        ht = 0;
        return;
    }

    image = ptr + dataOffset;

    // Some bitmaps have alpha channel data without a valid alpha bitmask.
    // In this case, scan the image for any nonzero alpha data in the unusued MSBs.
    // If all alpha bits are zero, the image is treated as fully opaque.
    if ((bitspp == 16 || bitspp == 32) && alphaChannel == false) {
        uint32_t unusedMask = (1 << bitspp) - (1 << (32 - __builtin_clz(rMask | gMask | bMask)));
        if (unusedMask != 0) {
            uint8_t* rowPtr = image;
            for (int j = 0; j < abs(ht); j++) {
                uint8_t* pixPtr = rowPtr;
                rowPtr += rowBytes;
                for (int i = 0; i < wd; i++) {
                    uint32_t pixWord;
                    if (bitspp == 16) {
                        pixWord = read16(pixPtr);
                        pixPtr += 2;
                    } else {
                        pixWord = read32(pixPtr);
                        pixPtr += 4;
                    }
                    if (pixWord & unusedMask) {
                        alphaChannel = true;
                        break;
                    }
                }
                if (alphaChannel == true) break;
            }
            if (alphaChannel == true) {
                aMask = unusedMask;
                aScale = maskToScale(aMask);
                aShift = maskToShift(aMask);
            }
        }
    }
}

uint8_t BitmapSprite::maskToScale(uint32_t mask) {
    // Calculate scale factor to convert color bits to standard 8 bit color
    // Formula: 8bitColor = ((pixelWord & mask) * scale) >> shift;
    uint8_t depth = __builtin_popcount(mask);
    uint8_t scale = 0;
    if (depth >= 8) {
        scale = 1;
    } else if (depth >= 4) {
        scale = (1 << depth) + 1;
    } else if (depth == 3) {
        scale = (1 << (2 * depth)) + (1 << depth) + 1; // 73
    } else if (depth == 2) {
        scale = (1 << (3 * depth)) + (1 << (2 * depth)) + (1 << depth) + 1; // 85
    } else if (depth == 1) {
        scale = 255;
    }
    return scale;
}

uint8_t BitmapSprite::maskToShift(uint32_t mask) {
    // Calculate bitshift to convert color bits to standard 8 bit color
    // Formula: 8bitColor = ((pixelWord & mask) * scale) >> shift;
    uint8_t depth = __builtin_popcount(mask);
    uint8_t shift = 0;
    if (depth >= 8) {
        shift = __builtin_ctz(mask) + depth - 8;
    } else if (depth >= 4) {
        shift = __builtin_ctz(mask) + (2 * depth) - 8;
    } else if (depth == 3) {
        shift = __builtin_ctz(mask) + (3 * depth) - 8;
    } else if (depth == 2) {
        shift = __builtin_ctz(mask) + (4 * depth) - 8;
    } else if (depth == 1) {
        shift = __builtin_ctz(mask) + (8 * depth) - 8;
    }
    return shift;
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
