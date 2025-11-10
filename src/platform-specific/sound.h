/*
  Platform specific sound functions
*/
#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>
#include <stdint.h>

void initSound();

void disableKeySounds();
void enableKeySounds();

/* Will bring back sound toggle if there is a platform with no controllable sound volume
bool toggleSound();
void setSound();
*/

void soundStop();
void soundJoinGame();
void soundMyTurn();
void soundGameDone();
void soundTick();
void soundCursor();
void soundPlaceShip();
void soundAttack();
void soundInvalid();
void soundHit();
void soundMiss();

void pause(unsigned char frames);

#endif /* SOUND_H */