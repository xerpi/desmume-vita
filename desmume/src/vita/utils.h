/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_W 960
#define SCREEN_H 544

#define align_mem(addr, align) (((addr) + ((align) - 1)) & ~((align) - 1))
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
#define abs(x) ((x) < 0 ? (-x) : (x))

#ifdef __cplusplus
}
#endif

#endif
