#ifndef HEX_FONT_H
#define HEX_FONT_H
#include <cstdint>

#define HEX_FONT_GLYPHS 16
#define HEX_FONT_WIDTH_BYTES 3
#define HEX_FONT_WIDTH 24
#define HEX_FONT_HEIGHT 24

extern const uint8_t HexFont24x24[HEX_FONT_GLYPHS][HEX_FONT_HEIGHT][HEX_FONT_WIDTH_BYTES];

#endif
