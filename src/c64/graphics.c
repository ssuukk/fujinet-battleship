/*
  Graphics functionality for Commodore 64
*/

#include <peekpoke.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "../platform-specific/graphics.h"
#include "vars.h"

extern unsigned char charset[];
extern void irqVsyncWait(void);

#define SCREEN_LOC ((uint8_t *)0xCC00)
#define COLOR_LOC ((uint8_t *)0xD800)
#define CHARSET_LOC 0xC000

#define xypos(x, y) (SCREEN_LOC + x + (y) * WIDTH)
#define colorpos(x, y) (COLOR_LOC + x + (y) * WIDTH)

// These started at 0x00
#define TOP_BORDER_START 0x25
#define TOP_BORDER_END 0x26
#define TILE_BORDER1_TOP_L 0x28
#define TILE_BORDER1_TOP_R 0x29
#define TILE_BORDER1_TOP 0x47 //(0xD2 + MAGIC_SHIFT2)
#define TILE_BORDER1_BOTTOM_L 0x2a
#define TILE_BORDER1_BOTTOM_R 0x2b
#define TILE_BORDER1_BOTTOM 0x49

// These seem to be at 40
#define TILE_BOTTOM_BORDER_START 0x40
#define TILE_BOTTOM_BORDER_END 0x41
#define TILE_TOP_BORDER 0x46
#define TILE_BOTTOM_BORDER 0x48

#define TILE_HIT 0x59 // 0x39
#define TILE_HIT2 0x3b // lub 9b, było 0x1B

#define TILE_HIT_LEGOND 0x3c // ???(0x2D + MAGIC_SHIFT3)   // -

// ????

#define TILE_SHIP_LEFT 0x52
#define TILE_SHIP_HULL_H 0x53
#define TILE_SHIP_RIGHT 0x54
#define TILE_SHIP_BOTTOM 0x55
#define TILE_SHIP_HULL_V 0x56
#define TILE_SHIP_TOP 0x57

#define TILE_TOP_LEFT_CORNET 0x5b // było 3b
#define TILE_TOP_RIGHT_CORNET 0x5c
#define TILE_BOTTOM_LEFT_CORNET 0x5d
#define TILE_BOTTOM_RIGHT_CORNET 0x5e

#define MAGIC_SHIFT4 32
#define TILE_CLOCK (0x43 + MAGIC_SHIFT4)
#define TILE_MISS (0x58 + MAGIC_SHIFT4)         // X
#define TILE_CURSOR (0x5B + MAGIC_SHIFT4)
// #define TILE_NAME_START (0x5C + MAGIC_SHIFT4)
// #define TILE_NAME_END (0x5D + MAGIC_SHIFT4)
// #define TILE_LABEL_START (0x5E + MAGIC_SHIFT4)
// #define TILE_LABEL_END (0x5F + MAGIC_SHIFT4)
#define TILE_NAME_START (0x1C)
#define TILE_NAME_END (0x1D)
#define TILE_LABEL_START (0x1E)
#define TILE_LABEL_END (0x1F)

#define MAGIC_SHIFT2 -64
#define TILE_SEA 0          // Space
#define TILE_LABEL (0x60 + MAGIC_SHIFT2)
#define TILE_VERTICAL_LINE (0x7C + MAGIC_SHIFT2)


// Color definitions (0-15 for C64)
#define COLOR_BORDER 0
#define COLOR_BG 6         // Blue
#define COLOR_CURSOR 1     // White
#define COLOR_SHIP 5       // Green
#define COLOR_HIT 2        // Red
#define COLOR_MISS 4       // Purple
#define COLOR_TEXT 1       // White

#define VIC_MEMORY_SETUP_REGISTER 0xD018
#define CIA2_VIDEO_BANK_REGISTER 0xDD00

static uint8_t colorMode = 0, colIndex = 0, fieldX = 0, playerCount = 0;
static bool inGameCharSet = false;

// Buffers to save/restore the visible screen (1000 bytes = 40x25)
static uint8_t saved_screen[1000];
static uint8_t saved_color[1000];
static bool saved_screen_valid = false;

// State for VIC bank switching to use RAM charset at CHARSET_LOC ($1000)
static uint8_t _saved_port01 = 0;
static uint8_t _saved_d018 = 0;

static uint16_t quadrant_offset[] = {
    WIDTH * 14 + 8,
    WIDTH * 2 + 8,
    WIDTH * 2 + 21,
    WIDTH * 14 + 21};

uint8_t legendShipOffset[] = {2, 1, 0, 40 * 5, 40 * 6 + 1};

// Defined in this file
void drawTextAdd(uint8_t *dest, const char *s, uint8_t add);
void drawShipInternal(uint8_t *dest, uint8_t size, uint8_t delta);

static void enableCustomCharset(void);
static void disableCustomCharset(void);

unsigned char toLowerCase(unsigned char c)
{
    if (c >= 97-32 && c <= 122-32) return c - 64;
    else return c;
};

unsigned char cycleNextColor()
{
    return 0;
};

void initGraphics()
{
    // Set up screen parameters
    // Put character set in RAM at $1000 if needed (most C64 programs use built-in ROM)
    // Copy included `charset` binary into RAM at `CHARSET_LOC` so custom glyphs are available.
    // NOTE: depending on your memory configuration you may also need to adjust the VIC-II
    // bank/registers (eg. $01 and $D018) to make this RAM visible to the video chip. Many
    // emulators and setups already allow $1000-$1FFF to be writable; if characters look
    // incorrect you'll need to enable the VIC to use RAM-based charset.
    // Ensure VIC is configured to use RAM charset, then copy charset into RAM
    enableCustomCharset();
    
    // Set border and background colors
    POKE(0xD020, COLOR_BORDER); // Border color
    POKE(0xD021, COLOR_BG);     // Background color
    // Enable multicolor character mode (set bit 4 in VIC register $D016)
    POKE(0xD016, PEEK(0xD016) | 0x10);
    
    // Clear screen memory
    memset(SCREEN_LOC, TILE_SEA, 1000);
    
    // Set all character colors to text color
    memset(COLOR_LOC, COLOR_TEXT, 1000);
}

void resetGraphics()
{
    // Reset to default colors
    POKE(0xD020, 254);
    POKE(0xD021, 246);
    // Restore previous memory mapping (if we changed it)
    disableCustomCharset();
    memset(SCREEN_LOC, TILE_SEA, 1000);
}

bool saveScreenBuffer()
{
    // Save screen (characters) and color RAM into local buffers
    memcpy(saved_screen, SCREEN_LOC, 1000);
    memcpy(saved_color, COLOR_LOC, 1000);
    saved_screen_valid = true;
    return true;
}

void restoreScreenBuffer()
{
    if (!saved_screen_valid)
        return;

    memcpy(SCREEN_LOC, saved_screen, 1000);
    memcpy(COLOR_LOC, saved_color, 1000);
    saved_screen_valid = false;
}

void drawText(unsigned char x, unsigned char y, const char *s)
{
    uint8_t *pos = xypos(x, y);
    uint8_t *col_pos = colorpos(x, y);
    unsigned char c;

    while ((c = *s++))
    {
        // Convert to uppercase PETSCII if needed
        c = toLowerCase(c);        
        *pos++ = c;
        *col_pos++ = COLOR_TEXT;
    }
}

void drawTextAlt(unsigned char x, unsigned char y, const char *s)
{
    uint8_t *pos = xypos(x, y);
    uint8_t *col_pos = colorpos(x, y);
    uint8_t c;
    uint8_t color;

    while ((c = *s++))
    {
        // Uppercase letters get alternate color
        if (c >= 65 && c <= 90)
        {
            color = COLOR_HIT;  // Alternate color
        }
        else
        {
            color = COLOR_TEXT;
        }
        
        c = toLowerCase(c);

        *pos++ = c;
        *col_pos++ = color;
    }
}

void resetScreen()
{
    memset(SCREEN_LOC, TILE_SEA, 1000);
    memset(COLOR_LOC, COLOR_TEXT, 1000);
}

void drawIcon(unsigned char x, unsigned char y, unsigned char icon)
{
    *xypos(x, y) = icon+32;
    *colorpos(x, y) = COLOR_CURSOR;
}

void drawBlank(unsigned char x, unsigned char y)
{
    *xypos(x, y) = TILE_SEA;
    *colorpos(x, y) = COLOR_BG;
}

void drawSpace(unsigned char x, unsigned char y, unsigned char w)
{
    memset(xypos(x, y), TILE_SEA, w);
    memset(colorpos(x, y), COLOR_BG, w);
}

void drawClock()
{
    POKE(xypos(WIDTH - 1, HEIGHT - 1), 0x9d);
    *colorpos(WIDTH - 1, HEIGHT - 1) = COLOR_CURSOR;
}

void drawConnectionIcon(bool show)
{
    POKEW(xypos(0, HEIGHT - 1), show ? 0x9f9e : 0);
    *colorpos(0, HEIGHT - 1) = COLOR_CURSOR;
}


void drawTextAdd(uint8_t *dest, const char *s, uint8_t add)
{
    uint8_t *col_dest = COLOR_LOC + (dest - SCREEN_LOC);
    char c;

    while ((c = *s++))
    {
        c = toLowerCase(c);
        
        *dest++ = c;
        *col_dest++ = add ? COLOR_HIT : COLOR_TEXT;
    }
}

void drawPlayerName(uint8_t player, const char *name, bool active)
{
    static uint8_t i, add;
    uint8_t *dest = SCREEN_LOC + fieldX + quadrant_offset[player] - WIDTH - 1;
    add = active ? 0 : 128;

    // Draw top and bottom borders and name label
    if (player == 0 || player == 3)
    {
        // Bottom player boards

        // Thin horizontal border
        dest[0] = 0x08 + add;
        dest[11] = 0x09 + add;
        memset(dest + 1, 0x27 + add, 10);

        // Name Label
        dest[WIDTH * 11] = 0x5E + add;
        dest[WIDTH * 11 + 11] = 0x5F + add;
        memset(WIDTH * 11 + 1 + dest, 0x60 + add, 10);
        drawTextAdd(dest + WIDTH * 11 + 2, name, add);

        // Active indicator
        if (active)
        {
            dest[WIDTH * 11 + 1] = 0x5b;
        }

        // Bottom border below name label
        dest[WIDTH * 12] = 0x20 + add;
        dest[WIDTH * 12 + 11] = 0x21 + add;
        memset(WIDTH * 12 + 1 + dest, 0x28 + add, 10);
    }
    else
    {
        // Top player boards

        // Top border above name label
        *(dest - WIDTH) = 0x5;
        *(dest - WIDTH + 11) = 0x6;
        memset(dest - WIDTH + 1, 0x26, 10);

        // Name Label
        dest[0] = 0x5C + add;
        dest[11] = 0x5D + add;
        memset(dest + 1, 0x60 + add, 10);
        drawTextAdd(dest + 2, name, add);

        // Active indicator
        if (active)
        {
            dest[1] = 0x5b;
        }

        // Thin horizontal border
        dest[WIDTH * 11] = 0x0A + add;
        dest[WIDTH * 11 + 11] = 0x0B + add;
        memset(WIDTH * 11 + 1 + dest, 0x29 + add, 10);
    }

    // Draw left/right borders and drawers
    if (player > 1 || playerCount == 2 && player > 0)
    {
        // Right drawer
        dest += WIDTH;
        memset(dest + 12, 0x31 + add, 3);
        dest[11] = 0x25 + add;
        dest[15] = 0x2D + add;
        dest[0] = 0x22 + add; // Left edge

        for (i = 0; i < 8; i++)
        {
            dest += WIDTH;
            dest[11] = 0x03 + add;
            dest[15] = 0x02 + add;
            dest[0] = 0x22 + add; // Left edge
        }

        dest += WIDTH;
        memset(dest + 12, 0x31 + add, 3);
        dest[11] = 0x25 + add;
        dest[15] = 0x2F + add;
        dest[0] = 0x22 + add; // Left edge
    }
    else
    {
        // Left drawer
        dest += WIDTH - 4;
        dest[0] = 0x2C + add;
        memset(dest + 1, 0x31 + add, 3);
        dest[4] = 0x24 + add;
        dest[15] = 0x23 + add; // Right edge

        for (i = 0; i < 8; i++)
        {
            dest += WIDTH;
            dest[0] = dest[4] = 0x02 + add;
            dest[15] = 0x23 + add; // Right edge
        }

        dest += WIDTH;
        dest[0] = 0x2E + add;
        memset(dest + 1, 0x31 + add, 3);
        dest[4] = 0x24 + add;
        dest[15] = 0x23 + add; // Right edge
    }
}

void drawBoard(uint8_t currentPlayerCount)
{
    static uint8_t i, y;
    static uint8_t *dest;
    playerCount = currentPlayerCount;
    fieldX = playerCount > 2 ? 0 : 7;

    if (playerCount > 1 && !inGameCharSet)
    {
        // Invert 0-9 & A-Z characters for in-game charset
        inGameCharSet = true;

        dest = CHARSET_LOC + 0x01 * 8;
        while (dest < CHARSET_LOC + 0x5b * 8)
        {
            *dest = *dest ^ 0xff | 0b01010101;
            dest++;
            if (dest == CHARSET_LOC + 0x1A * 8)
                dest = CHARSET_LOC + 0x40 * 8;
            if (dest == CHARSET_LOC + 0x02 * 8)
                dest = CHARSET_LOC + 0x10 * 8;
        }
    }

    for (i = 0; i < playerCount; i++)
    {
        dest = SCREEN_LOC + fieldX + quadrant_offset[i];

        // Draw player border
        drawPlayerName(i, "", false);

        // Blue gamefield
        for (y = 0; y < 10; y++)
        {
            memset(dest + y * WIDTH, TILE_SEA, 10);
        }

        // Blue drawer
        if (i > 1 || (i > 0 && fieldX > 0))
        {
            dest += WIDTH + 11;
        }
        else
        {
            dest += WIDTH - 4;
        }
        for (y = 0; y < 8; y++)
        {
            memset(dest, TILE_SEA, 3);
            dest += WIDTH;
        }
    }

    dest = xypos(16, 0);
}

void drawLine(unsigned char x, unsigned char y, unsigned char w)
{
    memset(xypos(x, y), 0x3F, w);
}

void drawShipInternal(uint8_t *dest, uint8_t size, uint8_t delta)
{
    static uint8_t i;
    uint8_t c = 0x32;
    uint8_t *src;
    if (delta)
        c = 0x37;

    for (i = 0; i < size; i++)
    {
        *dest = c;
        if (delta)
        {
            c = 0x36;
            if (i == size - 2)
                c = 0x35;
            dest += WIDTH;
        }
        else
        {
            dest++;
            c = 0x33;
            if (i == size - 2)
                c = 0x34;
        }
    }
}

void drawShip(uint8_t size, uint8_t pos, bool hide)
{
    uint8_t i, delta = 0;
    uint8_t *dest;

    if (pos > 99)
    {
        delta = 1; // 1=vertical, 0=horizontal
        pos -= 100;
    }

    dest = xypos((pos % 10), (pos / 10)) + fieldX + quadrant_offset[0];

    if (hide)
    {
        if (!delta)
            memset(dest, 0x38, size);
        else
            for (i = 0; i < size; i++)
                *(dest + i * WIDTH) = 0x38;
        return;
    }

    drawShipInternal(dest, size, delta);
}

void drawLegendShip(uint8_t player, uint8_t index, uint8_t size, uint8_t status)
{
    static uint8_t i;
    uint8_t *dest = SCREEN_LOC + fieldX + quadrant_offset[player] + legendShipOffset[index];

    if (player > 1 || (player > 0 && fieldX > 0))
    {
        dest += WIDTH + 11;
    }
    else
    {
        dest += WIDTH - 4;
    }

    if (status)
    {
        drawShipInternal(dest, size, 1);
    }
    else
    {
        for (i = 0; i < size; i++)
            *(dest + i * WIDTH) = TILE_HIT_LEGOND;
    }
}

void drawGamefield(uint8_t quadrant, uint8_t *field)
{
    static uint8_t y, x;
    uint8_t *dest = SCREEN_LOC + quadrant_offset[quadrant] + fieldX;

    for (y = 0; y < 10; ++y)
    {
        for (x = 0; x < 10; ++x)
        {
            if (*field)
            {
                *dest = *field == 1 ? TILE_HIT : TILE_MISS;
            }
            field++;
            dest++;
        }

        dest += WIDTH - 10;
    }
}

static bool cursorVisible = false;
void drawGamefieldUpdate(uint8_t quadrant, uint8_t *gamefield, uint8_t attackPos, uint8_t blink)
{
    uint8_t *dest = SCREEN_LOC + quadrant_offset[quadrant] + fieldX + (uint16_t)(attackPos / 10) * WIDTH + (attackPos % 10);
    uint8_t c = gamefield[attackPos];

    if (cursorVisible)
    {
        cursorVisible = false;
        //memset(PM_BASE + 1024, 0, 768);
    }

    // Animate attack (only in empty sea cell)
    if (blink > 9 && (*dest == TILE_SEA || *dest > 226))
    {
        *dest = 217 + blink;
        return;
    }

    if (c == FIELD_ATTACK)
    {
        *dest = blink ? TILE_HIT2 : TILE_HIT;
    }
    else if (c == FIELD_MISS)
    {
        *dest = TILE_MISS;
    }
    else
    {
        return;
    }
}
void drawGamefieldCursor(uint8_t quadrant, uint8_t x, uint8_t y, uint8_t *gamefield, uint8_t blink)
{
    uint16_t pos = quadrant_offset[quadrant] + y * WIDTH + fieldX + x;
    // uint16_t lc = lastCursor[quadrant];

    // lastCursor[quadrant] = PM_BASE + 768 + 24 + quadrant * 256 + (pos / WIDTH) * 8;

    // memset(lc, 0, 8);
    // memcpy(lastCursor[quadrant], &cursor_pmg, 8);

    // POKE(0xd000 - 1 + quadrant, (pos % WIDTH) * 4 + 48); // horiz loc
    cursorVisible = true;

    (void)gamefield;
}

void drawEndgameMessage(const char *message)
{
    uint8_t i, x;
    i = (uint8_t)strlen(message);
    x = WIDTH / 2 - i / 2;

    memset(xypos(0, HEIGHT - 2), 0x62, WIDTH);
    memset(xypos(0, HEIGHT - 1), 0x40, x);
    memset(xypos(x + i, HEIGHT - 1), 0x40, WIDTH - x - i);
    drawText(x, HEIGHT - 1, message);
}

void drawBox(unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
    uint8_t *pos = xypos(x, y);
    // Draw corners
    *pos = TILE_TOP_LEFT_CORNET;
    pos[w + 1] = TILE_TOP_RIGHT_CORNET;
    pos[h * WIDTH + WIDTH] = TILE_BOTTOM_LEFT_CORNET;
    pos[w + h * WIDTH + WIDTH + 1] = TILE_BOTTOM_RIGHT_CORNET;
}

void waitvsync()
{
    //irqVsyncWait();
}

// Enable VIC to use RAM-based charset at CHARSET_LOC. Saves previous $0001 and $D018.
static void enableCustomCharset(void)
{
    if (inGameCharSet)
        return;

    memcpy((void *)CHARSET_LOC, &charset, 2048);


    // _saved_port01 = PEEK(1);
    // _saved_d018 = PEEK(0xD018);

    // // Clear CHAREN (bit 2) in $01 so character ROM is not forced; keep other bits.
    // // This makes character ROM (if present) not visible to the VIC so RAM charset can be used.
    // POKE(1, _saved_port01 & ~0x04);

    // // Set character memory base in $D018. We set lower bits to point to $1000.
    // // The exact bit layout varies; this write attempts to set char base to $1000 while
    // // preserving the high bits. If your setup requires a different value, adjust accordingly.
    // POKE(0xD018, (_saved_d018 & 0xF8) | 0x04);

    // Configure the C64's memory layout for custom character set:
    // 1. Set up RAM bank for character ROM access
    // CIA2 port A (56576) controls RAM bank selection
    // Mask with 252 (11111100) to select the correct bank 3 for $C000    
    POKE(CIA2_VIDEO_BANK_REGISTER,PEEK(CIA2_VIDEO_BANK_REGISTER)&252);
    // 2. Configure VIC-II to use our custom character set
    // VIC_MEMORY_SETUP_REGISTER is the VIC-II control register for character ROM location
    // Setting to 32 tells VIC-II to use character ROM at $2000 (8192)
    // This points to our custom character set loaded at $C000
    // characters at offset 0, screen memory starts at $0c00
    POKE(VIC_MEMORY_SETUP_REGISTER,0x30); 
    // 3. Set the cursor character to use our custom character set
    // 648 is the cursor character location in memory
    // Setting to 200 makes cursor use character code 200 from our set
    //POKE(648,200);

    inGameCharSet = true;
}

// Restore previous memory mapping
static void disableCustomCharset(void)
{
    if (!inGameCharSet)
        return;

    POKE(0xD018, _saved_d018);
    POKE(1, _saved_port01);
    inGameCharSet = false;
}

