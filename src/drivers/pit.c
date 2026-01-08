/**
 * PIT (Programmable Interval Timer) Driver
 * Intel 8253/8254 timer chip - provides system timing
 */

#include <idt.h>
#include <kernel.h>
#include <pit.h>
#include <ports.h>

/* Current tick count (incremented by timer interrupt) */
static volatile uint32_t pit_ticks = 0;

/* Current timer frequency */
static uint32_t pit_frequency = 0;

/**
 * Timer interrupt handler (IRQ0)
 */
static void timer_handler(interrupt_frame_t *frame) {
    UNUSED(frame);
    pit_ticks++;
}

/**
 * Initialize the PIT with a given frequency
 * @param frequency: Desired timer frequency in Hz
 */
void pit_init(uint32_t frequency) {
    pit_set_frequency(frequency);
    pit_ticks = 0;

    /* Install timer interrupt handler */
    irq_install_handler(0, timer_handler);
}

/**
 * Set the PIT channel 0 frequency
 * @param frequency: Desired frequency in Hz (19-1193182)
 */
void pit_set_frequency(uint32_t frequency) {
    uint32_t divisor;

    /* Clamp frequency to valid range */
    if (frequency < 19) {
        frequency = 19;  /* Minimum frequency */
    }
    if (frequency > PIT_BASE_FREQUENCY) {
        frequency = PIT_BASE_FREQUENCY;
    }

    pit_frequency = frequency;

    /* Calculate divisor */
    divisor = PIT_BASE_FREQUENCY / frequency;

    /* Send command byte: Channel 0, lobyte/hibyte, rate generator, binary */
    outb(PIT_COMMAND, PIT_CHANNEL0 | PIT_LOHI | PIT_MODE2 | PIT_BINARY);

    /* Send divisor (low byte first, then high byte) */
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF));
}

/**
 * Get the current tick count
 * Note: Without interrupts, this won't increment automatically
 */
uint32_t pit_get_ticks(void) {
    return pit_ticks;
}

/**
 * Increment tick counter (call this from timer interrupt handler)
 */
void pit_tick(void) {
    pit_ticks++;
}

/**
 * Read the current count value from channel 0
 */
uint16_t pit_read_count(void) {
    uint16_t count;

    /* Latch the count */
    outb(PIT_COMMAND, PIT_CHANNEL0 | PIT_LATCH);

    /* Read low byte then high byte */
    count = inb(PIT_CHANNEL0_DATA);
    count |= (uint16_t)inb(PIT_CHANNEL0_DATA) << 8;

    return count;
}

/**
 * Busy-wait sleep using timer interrupts
 * @param ms: Milliseconds to sleep
 */
void pit_sleep(uint32_t ms) {
    uint32_t target_ticks;

    if (ms == 0) return;

    /* Calculate ticks needed: ticks = ms * frequency / 1000 */
    target_ticks = pit_ticks + (ms * pit_frequency) / 1000;

    /* Wait for ticks to reach target */
    /* Enable interrupts so timer can fire */
    while (pit_ticks < target_ticks) {
        __asm__ volatile ("sti; hlt");  /* Enable interrupts and wait */
    }
}

/**
 * Wait for a specific number of timer ticks
 * @param ticks: Number of ticks to wait
 */
void pit_wait_ticks(uint32_t ticks) {
    uint32_t target = pit_ticks + ticks;

    while (pit_ticks < target) {
        __asm__ volatile ("hlt");
    }
}
