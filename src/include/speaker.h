/**
 * PC Speaker Driver Header
 * Functions for controlling the PC speaker via PIT channel 2
 */

#ifndef SPEAKER_H
#define SPEAKER_H

#include "stdint.h"

/* PIT (Programmable Interval Timer) ports */
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND       0x43

/* Speaker control port */
#define SPEAKER_PORT      0x61

/* PIT base frequency (1.193182 MHz) */
#define PIT_FREQUENCY     1193182

/* Common musical note frequencies (Hz) */
#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_A4   440
#define NOTE_B4   494
#define NOTE_C5   523

#define NOTE_SYSTEM 1000

/* Function declarations */
void speaker_init(void);
void speaker_beep(uint32_t frequency, uint32_t duration_ms);
void speaker_stop(void);

#endif /* SPEAKER_H */
