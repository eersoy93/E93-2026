/**
 * PC Speaker Driver
 * Controls the PC speaker using PIT (Programmable Interval Timer) channel 2
 */

#include "speaker.h"
#include "pit.h"
#include "ports.h"

/**
 * Initialize the PC speaker
 */
void speaker_init(void) {
    /* Make sure speaker is off initially */
    speaker_stop();
}

/**
 * Play a tone at the specified frequency for a duration
 * @param frequency: Frequency in Hz (e.g., 440 for A4)
 * @param duration_ms: Duration in milliseconds (0 = play indefinitely)
 */
void speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    uint32_t divisor;
    uint8_t tmp;

    if (frequency == 0) {
        speaker_stop();
        return;
    }

    /* Calculate the PIT divisor */
    divisor = PIT_FREQUENCY / frequency;

    /* Set PIT channel 2 to square wave mode */
    outb(PIT_COMMAND, 0xB6);  /* Channel 2, lobyte/hibyte, square wave, binary */

    /* Set the frequency divisor */
    outb(PIT_CHANNEL2_DATA, (uint8_t)(divisor & 0xFF));         /* Low byte */
    outb(PIT_CHANNEL2_DATA, (uint8_t)((divisor >> 8) & 0xFF));  /* High byte */

    /* Enable the speaker */
    tmp = inb(SPEAKER_PORT);
    if ((tmp & 0x03) != 0x03) {
        outb(SPEAKER_PORT, tmp | 0x03);  /* Enable speaker (bits 0 and 1) */
    }

    /* If duration specified, wait and stop */
    if (duration_ms > 0) {
        pit_sleep(duration_ms);
        speaker_stop();
    }
}

/**
 * Stop the speaker
 */
void speaker_stop(void) {
    uint8_t tmp = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, tmp & 0xFC);  /* Disable speaker (clear bits 0 and 1) */
}
