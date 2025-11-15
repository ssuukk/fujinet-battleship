/*
  Graphics functionality
*/

#include <string.h>
#include "../platform-specific/graphics.h"
#include "../platform-specific/sound.h"
#include "../misc.h"

extern uint8_t charset[];
#define OFFSET_Y 2

#define ROP_CPY 0xff

// Mode 4
#define ROP_BLUE 0b10101010
#define ROP_YELLOW 0b01010101
#define BOX_SIDE 0b111100

#define COLOR_MODE_COCO3_RGB 1
#define COLOR_MODE_COCO3_COMPOSITE 2

#define LEGEND_X 24

// Defined in this file
void drawTextAltAt(uint8_t x, uint8_t y, const char *s);
void drawTextAt(uint8_t x, uint8_t y, const char *s);
void drawShipInternal(uint8_t *dest, uint8_t size, uint8_t delta);

extern char lastKey;
extern uint8_t background;
static uint8_t fieldX = 0;
int8_t highlightX = -1;
bool inBorderScreen = false;
uint8_t box_color = 0xff;
uint16_t quadrant_offset[] = {
    256U * 12 + 5 + 64,
    256U * 1 + 5 + 64,
    256U * 1 + 17 + 64,
    256U * 12 + 17 + 64};

uint16_t legendShipOffset[] = {2, 1, 0, 256U * 5, 256U * 6 + 1};

void updateColors()
{
}

uint8_t cycleNextColor()
{
}

void rgbOrComposite()
{
}

void initGraphics()
{
}

bool saveScreenBuffer()
{
    return false;
}

void restoreScreenBuffer()
{
    // No-op on CoCo
}

void drawPlayerName(uint8_t player, const char *name, bool active)
{
}

void drawText(uint8_t x, uint8_t y, const char *s)
{
}

void drawTextAt(uint8_t x, uint8_t y, const char *s)
{
}

void drawTextAlt(uint8_t x, uint8_t y, const char *s)
{
}

void drawTextAltAt(uint8_t x, uint8_t y, const char *s)
{
}

void resetScreen()
{
}

void drawLegendShip(uint8_t player, uint8_t index, uint8_t size, uint8_t status)
{
}

void drawGamefieldCursor(uint8_t quadrant, uint8_t x, uint8_t y, uint8_t *gamefield, uint8_t blink)
{
}

// Updates the gamefield display at attackPos
void drawGamefieldUpdate(uint8_t quadrant, uint8_t *gamefield, uint8_t attackPos, uint8_t blink)
{
}

void drawGamefield(uint8_t quadrant, uint8_t *field)
{
}

void drawShipInternal(uint8_t *dest, uint8_t size, uint8_t delta)
{
}

void drawShip(uint8_t size, uint8_t pos, bool hide)
{
}

void drawIcon(uint8_t x, uint8_t y, uint8_t icon)
{
}

void drawClock()
{
}

void drawConnectionIcon(bool show)
{
}

void drawSpace(uint8_t x, uint8_t y, uint8_t w)
{
}

void drawBoard(uint8_t playerCount)
{
}

void drawLine(uint8_t x, uint8_t y, uint8_t w)
{
}

void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
}

void resetGraphics()
{
}

void waitvsync()
{
}

void drawBlank(uint8_t x, uint8_t y)
{
}
