#ifndef gammaLUT_h
#define gammaLUT_h

extern const uint16_t gammaLUT8to16[];

extern const uint8_t invGammaLUT12to8[];

inline uint8_t encodeGamma16to8(uint16_t c) {
    return invGammaLUT12to8[c>>4];
}

inline uint16_t decodeGamma8to16(uint8_t c) {
    return gammaLUT8to16[c];
}

#endif
