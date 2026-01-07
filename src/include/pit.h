/**
 * PIT (Programmable Interval Timer) Driver Header
 * Intel 8253/8254 timer chip interface
 */

#ifndef PIT_H
#define PIT_H

#include "stdint.h"

/* PIT I/O ports */
#define PIT_CHANNEL0_DATA 0x40  /* Channel 0 data port (IRQ0 timer) */
#define PIT_CHANNEL1_DATA 0x41  /* Channel 1 data port (legacy DRAM refresh) */
#define PIT_CHANNEL2_DATA 0x42  /* Channel 2 data port (PC speaker) */
#define PIT_COMMAND       0x43  /* Mode/command register */

/* PIT base frequency: 1.193182 MHz */
#define PIT_BASE_FREQUENCY 1193182

/* PIT command byte bits */
#define PIT_CHANNEL0      (0 << 6)  /* Select channel 0 */
#define PIT_CHANNEL1      (1 << 6)  /* Select channel 1 */
#define PIT_CHANNEL2      (2 << 6)  /* Select channel 2 */
#define PIT_READBACK      (3 << 6)  /* Read-back command */

#define PIT_LATCH         (0 << 4)  /* Latch count value */
#define PIT_LOBYTE        (1 << 4)  /* Access low byte only */
#define PIT_HIBYTE        (2 << 4)  /* Access high byte only */
#define PIT_LOHI          (3 << 4)  /* Access low byte then high byte */

#define PIT_MODE0         (0 << 1)  /* Interrupt on terminal count */
#define PIT_MODE1         (1 << 1)  /* Hardware re-triggerable one-shot */
#define PIT_MODE2         (2 << 1)  /* Rate generator */
#define PIT_MODE3         (3 << 1)  /* Square wave generator */
#define PIT_MODE4         (4 << 1)  /* Software triggered strobe */
#define PIT_MODE5         (5 << 1)  /* Hardware triggered strobe */

#define PIT_BINARY        0         /* 16-bit binary counter */
#define PIT_BCD           1         /* 4-digit BCD counter */

/* Default timer frequency (1000 Hz = 1ms per tick) */
#define PIT_DEFAULT_FREQ  1000

/* Function declarations */
void pit_init(uint32_t frequency);
void pit_set_frequency(uint32_t frequency);
uint32_t pit_get_ticks(void);
void pit_sleep(uint32_t ms);
void pit_wait_ticks(uint32_t ticks);

/* Read current channel 0 count */
uint16_t pit_read_count(void);

#endif /* PIT_H */
