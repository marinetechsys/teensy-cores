/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "EventResponder.h"
#include "core_pins.h"
#include "arm_math.h"	// micros() synchronization

volatile uint8_t yield_active_check_flags = 0;

extern "C" {
extern volatile uint32_t systick_millis_count;
extern volatile uint32_t systick_cycle_count;
extern volatile uint32_t scale_cpu_cycles_to_microseconds;
extern uint32_t systick_safe_read;
} // extern C


#pragma GCC push_options
#pragma GCC optimize ("-fno-unwind-tables", "-fno-asynchronous-unwind-tables", "-fno-exceptions")

FASTRUN uint64_t get_us() {
    uint32_t smc, scc, cyccnt;
    do {
        __LDREXW(&systick_safe_read);
        smc = systick_millis_count;
        scc = systick_cycle_count;
        cyccnt = ARM_DWT_CYCCNT;
    } while (__STREXW(1, &systick_safe_read));

    const uint32_t ccdelta { cyccnt - scc };
    const uint32_t frac { static_cast<uint32_t>((static_cast<uint64_t>(ccdelta) * scale_cpu_cycles_to_microseconds) >> 32) };

    return static_cast<uint64_t>(smc) * 1'000ULL + frac;
}

FASTRUN uint64_t get_us_from_isr() {
    const uint32_t smc { systick_millis_count };
    const uint32_t scc { systick_cycle_count };
    const uint32_t cyccnt { ARM_DWT_CYCCNT };
    const uint32_t ccdelta { cyccnt - scc };
    const uint32_t frac { static_cast<uint32_t>((static_cast<uint64_t>(ccdelta) * scale_cpu_cycles_to_microseconds) >> 32) };

    return static_cast<uint64_t>(smc) * 1'000ULL + frac;
}

#pragma GCC pop_options

void yield(void) __attribute__ ((weak));
void yield(void)
{
	const uint8_t check_flags = yield_active_check_flags;
	if (!check_flags) return;	// nothing to do

	// TODO: do nothing if called from interrupt

	static uint8_t running=0;
	if (running) return; // TODO: does this need to be atomic?
	running = 1;

	// USB Serial - Add hack to minimize impact...
	if (check_flags & YIELD_CHECK_USB_SERIAL) {
		if (Serial.available()) serialEvent();
	}

#if defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL)
	if (check_flags & YIELD_CHECK_USB_SERIALUSB1) {
		if (SerialUSB1.available()) serialEventUSB1();
	}
#endif
#ifdef USB_TRIPLE_SERIAL
	if (check_flags & YIELD_CHECK_USB_SERIALUSB2) {
		if (SerialUSB2.available()) serialEventUSB2();
	}
#endif

	// Current workaround until integrate with EventResponder.
	if (check_flags & YIELD_CHECK_HARDWARE_SERIAL) {
		HardwareSerialIMXRT::processSerialEventsList();
	}

	running = 0;
	if (check_flags & YIELD_CHECK_EVENT_RESPONDER) {
		EventResponder::runFromYield();
	}
};
