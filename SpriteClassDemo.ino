// todo: compatibility with T3.6

#include "MatrixHardware_Teensy4_ShieldV5.h"

#include "BitmapSprite.h"
#include <SD.h>

#include <SmartMatrix.h>

// SmartMatrix setup parameters
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint16_t kMatrixWidth = 128;
const uint16_t kMatrixHeight = 64;
const uint8_t kRefreshDepth = 30;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
uint16_t refreshRate = 240;

// Allocate SmartMatrix buffers
SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#define NUMSPRITES 50
BitmapSprite sprites[NUMSPRITES];

int spriteXhome[NUMSPRITES];
int spriteYhome[NUMSPRITES];
int spriteXampl[NUMSPRITES];
int spriteYampl[NUMSPRITES];
float spriteXphase[NUMSPRITES];
float spriteYphase[NUMSPRITES];
float spriteAphase[NUMSPRITES];

void setup() {
    Serial.begin(9600);

    SD.begin(BUILTIN_SDCARD);

    delay(1000);

    matrix.addLayer(&backgroundLayer);
    matrix.begin();
    matrix.setRefreshRate(refreshRate);

    /* This line of code is needed for the Sprite class to function */
    BitmapSprite::setDisplaySize(kMatrixWidth, kMatrixHeight);

    /* Allocate sprite memory statically:
     *  dataSize must be large enough to fit the whole .bmp file */
    const size_t dataSize = 922;
    static uint8_t data[dataSize];
    sprites[0] = BitmapSprite("heart.bmp", data, dataSize);

    /* Alternatively: Allocate sprite memory dynamically as needed */
    // sprites[0] = BitmapSprite("heart.bmp");

    for (int i = 0; i < NUMSPRITES; i++) {
        // Copy the first sprite to create multiple independent sprites that all use the same image
        // Note: doing this does not allocate more memory 
        sprites[i] = sprites[0];

        // create some random data to define sprite motions and fades
        spriteXhome[i] = random(0, kMatrixWidth);
        spriteYhome[i] = random(0, kMatrixHeight);
        spriteXampl[i] = random(-8, 8);
        spriteYampl[i] = random(-8, 8);
        spriteXphase[i] = ((float)random(0, 100)) / 100.0F;
        spriteYphase[i] = ((float)random(0, 100)) / 100.0F;
        spriteAphase[i] = ((float)random(0, 100)) / 100.0F;
    }
}

void loop() {

    // Get the drawing buffer pointer
    rgb24* matrixBuffer = backgroundLayer.backBuffer();

    // Clear screen
    memset(matrixBuffer, 0, kMatrixHeight * kMatrixWidth * sizeof(rgb24));

    uint period = 4000;
    float fraction = ((float)(millis() % period)) / ((float)period);

    for (int i = 0; i < NUMSPRITES; i++) {
        // Compute sprite motion and fade
        sprites[i].x = spriteXhome[i] + roundf(spriteXampl[i] * cosf(6.2831853F * (fraction + spriteXphase[i])));
        sprites[i].y = spriteYhome[i] + roundf(spriteYampl[i] * cosf(6.2831853F * (fraction + spriteYphase[i])));
        sprites[i].alpha = roundf((255 + 255 * cosf(6.2831853F * (fraction + spriteAphase[i]))) / 2);

        // Render sprite to drawing buffer
        sprites[i].render(matrixBuffer);
    }

    backgroundLayer.swapBuffers(false);
}
