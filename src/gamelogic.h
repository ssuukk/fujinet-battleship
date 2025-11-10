#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "platform-specific/graphics.h"

void processStateChange();
void renderLobby();
void renderGameboard();
void handleAnimation();

void processInput();

void clearRenderState();

void centerText(unsigned char y, const char *text);
void centerTextAlt(unsigned char y, const char *text);
void centerTextWide(unsigned char y, const char *text);
void centerStatusText(const char *text);

void resetInputField();
bool inputFieldCycle(uint8_t x, uint8_t y, uint8_t max, char *buffer);

void waitOnPlayerMove();

void progressAnim(unsigned char y);

void placeShip(uint8_t shipSize, uint8_t pos);
bool testShip(uint8_t shipSize, uint8_t pos);

#endif /*GAMELOGIC_H*/
