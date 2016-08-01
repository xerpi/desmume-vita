/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void console_init();
void console_fini();
void console_print(const char *s);
void console_printf(const char *s, ...);

#define DBG(...) console_printf(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
