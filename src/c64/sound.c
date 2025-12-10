/*
  Platform specific sound functions for Commodore 64
*/

#include <stdint.h>
#include <stdlib.h>
#include <peekpoke.h>
#include "../misc.h"
#include "../platform-specific/sound.h"

// C64 SID chip registers
#define SID_BASE 0xD400

#define SID_FREQ_LO 0xD400
#define SID_FREQ_HI 0xD401
#define SID_PW_LO 0xD402
#define SID_PW_HI 0xD403
#define SID_CONTROL 0xD404
#define SID_ATTACK_DECAY 0xD405
#define SID_SUSTAIN_RELEASE 0xD406

#define SID_FILTER_FREQ_LO 0xD415
#define SID_FILTER_FREQ_HI 0xD416
#define SID_FILTER_RES_FILT 0xD417
#define SID_FILTER_MODE_VOL 0xD418

// Gate bit in control register
#define GATE_ON 0x01
#define GATE_OFF 0x00

// Waveform bits
#define WAVEFORM_TRIANGLE 0x10
#define WAVEFORM_SAWTOOTH 0x20
#define WAVEFORM_PULSE 0x40
#define WAVEFORM_NOISE 0x80

uint16_t ii;

void initSound()
{
    POKE(SID_FILTER_MODE_VOL, 0x0F);  // Set volume to max, no filter
}

void playTone(uint8_t frequency, uint8_t duration, uint8_t waveform)
{
    if (prefs.disableSound)
        return;

    // Set frequency
    POKE(SID_FREQ_LO, frequency & 0xFF);
    POKE(SID_FREQ_HI, (frequency >> 8) & 0xFF);

    // Set waveform and gate on
    POKE(SID_CONTROL, waveform | GATE_ON);

    // Play for duration (in frames approximately)
    while (duration--)
        for (ii = 0; ii < 100; ii++)
            ;

    // Gate off (stop sound)
    POKE(SID_CONTROL, waveform);
}

void soundCursor()
{
    if (prefs.disableSound)
        return;

    playTone(100, 2, WAVEFORM_PULSE);
    playTone(120, 2, WAVEFORM_PULSE);
    playTone(140, 2, WAVEFORM_PULSE);
}

void soundSelect()
{
    if (prefs.disableSound)
        return;

    playTone(150, 3, WAVEFORM_PULSE);
}

void soundStop()
{
    // Stop any playing sound
    POKE(SID_CONTROL, 0);
}

void soundJoinGame()
{
    if (prefs.disableSound)
        return;

    playTone(80, 2, WAVEFORM_TRIANGLE);
    playTone(60, 2, WAVEFORM_TRIANGLE);
    playTone(100, 3, WAVEFORM_TRIANGLE);
}

void soundMyTurn()
{
    if (prefs.disableSound)
        return;

    playTone(120, 1, WAVEFORM_PULSE);
    playTone(120, 2, WAVEFORM_PULSE);
}

void soundGameDone()
{
    uint8_t i;
    if (prefs.disableSound)
        return;

    for (i = 0; i < 3; i++)
    {
        playTone(200, 2, WAVEFORM_TRIANGLE);
        playTone(100, 2, WAVEFORM_TRIANGLE);
    }
    playTone(150, 5, WAVEFORM_TRIANGLE);
}

void soundTick()
{
    if (prefs.disableSound)
        return;

    playTone(160, 1, WAVEFORM_PULSE);
}

void soundPlaceShip()
{
    uint8_t i;
    if (prefs.disableSound)
        return;

    for (i = 0; i < 5; i++)
    {
        playTone(100 + (i * 10), 1, WAVEFORM_TRIANGLE);
    }
}

void soundAttack()
{
    uint8_t i;
    if (prefs.disableSound)
        return;

    // Descending tone for attack
    for (i = 200; i > 100; i -= 10)
    {
        playTone(i, 1, WAVEFORM_SAWTOOTH);
    }
}

void soundInvalid()
{
    if (prefs.disableSound)
        return;

    // Buzzer sound for invalid action
    playTone(50, 1, WAVEFORM_NOISE);
    playTone(40, 1, WAVEFORM_NOISE);
    playTone(30, 1, WAVEFORM_NOISE);
}

void soundHit()
{
    uint8_t i;
    if (prefs.disableSound)
        return;

    // Ascending tones for hit
    for (i = 100; i < 200; i += 20)
    {
        playTone(i, 1, WAVEFORM_TRIANGLE);
    }
}

void soundMiss()
{
    if (prefs.disableSound)
        return;

    playTone(80, 2, WAVEFORM_PULSE);
    playTone(60, 2, WAVEFORM_PULSE);
}

void disableKeySounds()
{
}

void enableKeySounds()
{
}
