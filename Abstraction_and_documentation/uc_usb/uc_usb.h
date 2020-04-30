/* libusbcdc header
 * Warren W. Gay VE3WWG
 */
#ifndef LIBUSBCDC_H
#define LIBUSBCDC_H

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void usb_start(void);
void usb_putc(char ch);
void usb_puts(char* str);
int usb_printf(const char *format, ...);
void usb_msg_append(char ch);


#endif /* LIBUSBCDC_H */

/* End libusbcdc.h */
