/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2024
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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HardwareSerialIMXRT;

typedef int (*usb_serial_redirect_cb_t)(void *context, uint8_t data);

struct usb_serial_redirect_adapter {
	usb_serial_redirect_cb_t callback;
	void *context;
};

size_t usb_serial_bridge_uart_write(struct HardwareSerialIMXRT *uart, const uint8_t *buffer, size_t size);
void usb_serial_bridge_set_redirect(struct HardwareSerialIMXRT *uart, usb_serial_redirect_cb_t cb, void *context);
void usb_serial_bridge_clear_redirect(struct HardwareSerialIMXRT *uart, void *context);
struct HardwareSerialIMXRT *usb_serial_bridge_get_uart(uint8_t index);
uint8_t usb_serial_bridge_get_uart_index(struct HardwareSerialIMXRT *uart);
void usb_serial_bridge_uart_begin(struct HardwareSerialIMXRT *uart, uint32_t baud, uint16_t format);
uint16_t usb_serial_bridge_format_from_line_coding(uint32_t line_format);

#ifdef __cplusplus
}
#endif
