#ifndef BLEND_H
#define BLEND_H
#include "include/GColor.h"
#include "include/GPixel.h"
#include "include/GPaint.h"
#include "include/GBlendMode.h"

GPixel GColorToPixel(const GColor& color);
uint64_t const GExpand(uint32_t op);

uint32_t GDiv255(uint64_t op);
uint64_t GMultXor(GPixel c1, GPixel x1, GPixel c2, GPixel x2);
uint32_t GDiv(uint32_t op);
GPixel GMultPixel(const GPixel& p1, const GPixel& p2);
uint64_t GMult(uint32_t op1, uint8_t op2);
uint32_t GMult2(uint32_t op1, uint8_t op2);
GPixel blend_clear(const GPixel& src, const GPixel& des);

    GPixel blend_src(const GPixel& src, const GPixel& des);

    GPixel blend_dst(const GPixel& src, const GPixel& des);
    GPixel blend_srcover(const GPixel& src, const GPixel& des);
    GPixel blend_dstover(const GPixel& src, const GPixel& des);
    GPixel blend_srcin(const GPixel& src, const GPixel& des);
    GPixel blend_dstin(const GPixel& src, const GPixel& des);

    GPixel blend_srcout(const GPixel& src, const GPixel& des);
    GPixel blend_dstout(const GPixel& src, const GPixel& des);
    GPixel blend_srcatop(const GPixel& src, const GPixel& des);
    GPixel blend_dstatop(const GPixel& src, const GPixel& des);
    GPixel blend_xor(const GPixel& src, const GPixel& des);
    

void GBlend(const GPaint& paint, GPixel row[], int count);
void GShade(const GPaint& paint, GPixel* row, int x, int y, int count);
#endif