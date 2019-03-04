#ifndef __SHAPEFILE_INTERNAL_H__
#define __SHAPEFILE_INTERNAL_H__

#include "Shapefile.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*  Utility functions. */
int32_t byteswap32(int32_t value);
void print_msg(const char* format, ...);

/*  Shape file functions. */
SFShapes* allocate_shapes(FILE* shapefile);

#ifdef __cplusplus
}
#endif

/*    __SHAPEFILE_INTERNAL_H__ */
#endif