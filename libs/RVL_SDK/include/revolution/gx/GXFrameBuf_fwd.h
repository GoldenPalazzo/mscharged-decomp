#ifndef RVL_SDK_GX_FRAMEBUF_FWD_H
#define RVL_SDK_GX_FRAMEBUF_FWD_H

#include <revolution/gx/GXRenderModeObj.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXSetCopyFilter(unsigned char aa, const unsigned char sample_pattern[12][2],
                     unsigned char vf, const unsigned char vfilter[GX_VFILTER_SZ]);

#ifdef __cplusplus
}
#endif

#endif
