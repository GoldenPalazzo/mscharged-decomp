#ifndef RVL_SDK_GX_PUBLIC_TYPES_H
#define RVL_SDK_GX_PUBLIC_TYPES_H

typedef unsigned char GXBool;

typedef struct _GXColor {
    unsigned char r, g, b, a;
} GXColor;

typedef enum _GXCompare {
    GX_NEVER,
    GX_LESS,
    GX_EQUAL,
    GX_LEQUAL,
    GX_GREATER,
    GX_NEQUAL,
    GX_GEQUAL,
    GX_ALWAYS
} GXCompare;

#endif // RVL_SDK_GX_PUBLIC_TYPES_H
