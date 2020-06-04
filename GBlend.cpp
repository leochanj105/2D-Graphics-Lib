#include "GBlend.h"
#include "include/GPixel.h"
#include "include/GPaint.h"
#include "include/GColor.h"
#include "include/GShader.h"
#include "include/GBlendMode.h"

// enum class GMode{
//     mNormal,
//     mZero,
//     mHalf,
//     mOne
// };

// typedef GPixel (*GBlendFunc)(const GPixel&, const GPixel&);

GPixel GColorToPixel(const GColor& color){
    float fAPinned = GPinToUnit(color.fA);
    return GPixel_PackARGB(GRoundToInt(fAPinned * 255.0),
            GRoundToInt(GPinToUnit(color.fR) * fAPinned * 255.0),
            GRoundToInt(GPinToUnit(color.fG) * fAPinned * 255.0),
            GRoundToInt(GPinToUnit(color.fB) * fAPinned * 255.0));
}
uint64_t const GExpand(uint32_t op){
      uint64_t hi = op & 0xFF00FF00;
      uint64_t lo = op & 0x00FF00FF;
      return (hi << 24) | lo;
}

uint32_t GDiv255(uint64_t op){
      op >>= 8;
      return ((op >> 24) & 0xFF00FF00) | (op & 0xFF00FF);
}
uint64_t GMultXor(GPixel c1, GPixel x1, GPixel c2, GPixel x2){
      uint64_t result = GExpand(x1) * c1 + GExpand(x2) * c2;
      result += 0x80008000800080;
      result += (result >> 8) & 0xFF00FF00FF00FF;
      return result;
}
uint32_t GDiv(uint32_t op){
  op += 128;
  return op + (op >> 8) >> 8;
}
GPixel GMultPixel(const GPixel& p1, const GPixel& p2){
  int a1 = GPixel_GetA(p1), r1 = GPixel_GetR(p1), g1 = GPixel_GetG(p1), b1 = GPixel_GetB(p1);
  int a2 = GPixel_GetA(p2), r2 = GPixel_GetR(p2), g2 = GPixel_GetG(p2), b2 = GPixel_GetB(p2);
  return GPixel_PackARGB(GDiv(a1* a2), GDiv(r1* r2), GDiv(g1* g2), GDiv(b1* b2));
}
uint64_t GMult(uint32_t op1, uint8_t op2){
      uint64_t result = GExpand(op1) * op2;
      result += 0x80008000800080;
      result += (result >> 8) & 0xFF00FF00FF00FF;
      return result;
}

    GPixel div2_127(const GPixel& pixel){
      return GPixel_PackARGB((GPixel_GetA(pixel)) >> 1, 
                             (GPixel_GetR(pixel)) >> 1, 
                             (GPixel_GetG(pixel)) >> 1, 
                             (GPixel_GetB(pixel)) >> 1);
    }

    GPixel div2_128(const GPixel& pixel){
      return GPixel_PackARGB((GPixel_GetA(pixel) + 1) >> 1, 
                             (GPixel_GetR(pixel) + 1) >> 1, 
                             (GPixel_GetG(pixel) + 1) >> 1, 
                             (GPixel_GetB(pixel) + 1) >> 1);
    }

    GPixel blend_clear(const GPixel& src, const GPixel& des){
      return 0;
    }

    GPixel blend_src(const GPixel& src, const GPixel& des){
      return src;
    }

    GPixel blend_dst(const GPixel& src, const GPixel& des){
      return des;
    }
    GPixel blend_srcover(const GPixel& src, const GPixel& des){
      return src + GDiv255(GMult(des, 255 - GPixel_GetA(src)));
    }
    GPixel blend_dstover(const GPixel& src, const GPixel& des){
      return des + GDiv255(GMult(src, 255 - GPixel_GetA(des)));
    }
    GPixel blend_srcin(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(src, GPixel_GetA(des)));
    }
    GPixel blend_dstin(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(des, GPixel_GetA(src)));
    }

    GPixel blend_srcout(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(src, 255 - GPixel_GetA(des)));
    }
    GPixel blend_dstout(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(des, 255 - GPixel_GetA(src)));
    }
    GPixel blend_srcatop(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(src, GPixel_GetA(des))) + GDiv255(GMult(des, 255 - GPixel_GetA(src)));
    }
    GPixel blend_dstatop(const GPixel& src, const GPixel& des){
      return GDiv255(GMult(des, GPixel_GetA(src))) + GDiv255(GMult(src, 255 - GPixel_GetA(des)));
    }
    GPixel blend_xor(const GPixel& src, const GPixel& des){
      return GDiv255(GMultXor(255 - GPixel_GetA(src), des, 255 - GPixel_GetA(des), src));
    }
    

void GBlend(const GPaint& paint, GPixel* row, int count){
      GBlendMode mode = paint.getBlendMode();
      GColor color = paint.getColor();
      GPixel src = GColorToPixel(color);
      float fA = color.fA;
      if(fA == 0.0){
        switch(mode){
          case GBlendMode::kClear:
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrc: 
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kDst:
            break;
          case GBlendMode::kSrcOver:
            break;
          case GBlendMode::kDstOver:
            break;
          case GBlendMode::kSrcIn: 
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kDstIn: 
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrcOut:
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kDstOut:  
            break;
          case GBlendMode::kSrcATop: 
            break;
          case GBlendMode::kDstATop: 
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kXor:{
            break;
          }
        }
      }
      else if(fA == 1.0){
        switch(mode){
          case GBlendMode::kClear:
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrc: 
            std::fill(row, row + count, src);
            break;
          case GBlendMode::kDst:
            break;
          case GBlendMode::kSrcOver:
            std::fill(row, row + count, src);
            break;
          case GBlendMode::kDstOver:
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstover(src, row[i]);
            break;
          case GBlendMode::kSrcIn: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcin(src, row[i]);
            break;
          case GBlendMode::kDstIn: 
            break;
          case GBlendMode::kSrcOut:
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]);
            break;
          case GBlendMode::kDstOut:  
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrcATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcin(src, row[i]);
            break;
          case GBlendMode::kDstATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]) + row[i];
            break;
          case GBlendMode::kXor:{
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]);
            break;
          }
        }
      }
      else if(fA == 0.5){
        switch(mode){
          case GBlendMode::kClear:
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrc: 
            std::fill(row, row + count, src);
            break;
          case GBlendMode::kDst:
            break;
          case GBlendMode::kSrcOver:
            for(int i = 0; i < count; ++i)
              row[i] = src + div2_127(row[i]);
            break;
          case GBlendMode::kDstOver:
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstover(src, row[i]);
            break;
          case GBlendMode::kSrcIn: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcin(src, row[i]);
            break;
          case GBlendMode::kDstIn: 
            for(int i = 0; i < count; ++i)
              row[i] = div2_128(row[i]);
            break;
          case GBlendMode::kSrcOut:
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]);
            break;
          case GBlendMode::kDstOut:  
            for(int i = 0; i < count; ++i)
              row[i] = div2_127(row[i]);
            break;
          case GBlendMode::kSrcATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcin(src, row[i]) + div2_127(row[i]);
            break;
          case GBlendMode::kDstATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]) + div2_128(row[i]);
            break;
          case GBlendMode::kXor:{
            for(int i = 0; i < count; ++i)
              row[i] = blend_xor(src, row[i]);
            break;
          }
        }
      }
      else{
        switch(mode){
          case GBlendMode::kClear:
            std::fill(row, row + count, 0);
            break;
          case GBlendMode::kSrc: 
            std::fill(row, row + count, src);
            break;
          case GBlendMode::kDst:
            break;
          case GBlendMode::kSrcOver:
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcover(src, row[i]);
            break;
          case GBlendMode::kDstOver:
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstover(src, row[i]);
            break;
          case GBlendMode::kSrcIn: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcin(src, row[i]);
            break;
          case GBlendMode::kDstIn: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstin(src, row[i]);
            break;
          case GBlendMode::kSrcOut:
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcout(src, row[i]);
            break;
          case GBlendMode::kDstOut:  
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstout(src, row[i]);
            break;
          case GBlendMode::kSrcATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_srcatop(src, row[i]);
            break;
          case GBlendMode::kDstATop: 
            for(int i = 0; i < count; ++i)
              row[i] = blend_dstatop(src, row[i]);
            break;
          case GBlendMode::kXor:{
            for(int i = 0; i < count; ++i)
              row[i] = blend_xor(src, row[i]);
            break;
          }
        }
      }
    }
    void GShade(const GPaint& paint, GPixel* row, int x, int y, int count){
      
      if(paint.getShader() == nullptr){
        GBlend(paint, row, count);
      }
      else{
         GShader* shader = paint.getShader();
         GPixel src[count];
         shader->shadeRow(x, y, count, src);
         GBlendMode mode = paint.getBlendMode();
        if(shader -> isOpaque()){
          switch(mode){
            case GBlendMode::kClear:
              std::fill(row, row + count, 0);
              break;
            case GBlendMode::kSrc: 
              for(int i = 0; i < count; ++i)
                row[i] = src[i];
              break;
            case GBlendMode::kDst:
              break;
            case GBlendMode::kSrcOver:
              for(int i = 0; i < count; ++i)
                row[i] = src[i];
              break;
            case GBlendMode::kDstOver:
              for(int i = 0; i < count; ++i)
                row[i] = blend_dstover(src[i], row[i]);
              break;
            case GBlendMode::kSrcIn: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcin(src[i], row[i]);
              break;
            case GBlendMode::kDstIn:
              break;
            case GBlendMode::kSrcOut:
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcout(src[i], row[i]);
              break;
            case GBlendMode::kDstOut:  
              std::fill(row, row + count, 0);
              break;
            case GBlendMode::kSrcATop: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcin(src[i], row[i]);
              break;
            case GBlendMode::kDstATop: 
              for(int i = 0; i < count; ++i)
                row[i] = row[i] + blend_srcout(src[i], row[i]);
              break;
            case GBlendMode::kXor:{
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcout(src[i], row[i]);
              break;
            }
          }
        }
        else{
          switch(mode){
            case GBlendMode::kClear:
              std::fill(row, row + count, 0);
              break;
            case GBlendMode::kSrc: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_src(src[i], row[i]);
              break;
            case GBlendMode::kDst:
              break;
            case GBlendMode::kSrcOver:
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcover(src[i], row[i]);
              break;
            case GBlendMode::kDstOver:
              for(int i = 0; i < count; ++i)
                row[i] = blend_dstover(src[i], row[i]);
              break;
            case GBlendMode::kSrcIn: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcin(src[i], row[i]);
              break;
            case GBlendMode::kDstIn: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_dstin(src[i], row[i]);
              break;
            case GBlendMode::kSrcOut:
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcout(src[i], row[i]);
              break;
            case GBlendMode::kDstOut:  
              for(int i = 0; i < count; ++i)
                row[i] = blend_dstout(src[i], row[i]);
              break;
            case GBlendMode::kSrcATop: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_srcatop(src[i], row[i]);
              break;
            case GBlendMode::kDstATop: 
              for(int i = 0; i < count; ++i)
                row[i] = blend_dstatop(src[i], row[i]);
              break;
            case GBlendMode::kXor:{
              for(int i = 0; i < count; ++i)
                row[i] = blend_xor(src[i], row[i]);
              break;
            }
          }
        }
      }
    }

    

    