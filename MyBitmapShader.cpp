
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"


class MyBitmapShader: public GShader{

public:
    
    MyBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tile): 
                                    bitmap(bitmap), local(localMatrix), tile(tile){}
    bool isOpaque() override{
        return bitmap.isOpaque();
    }

    bool setContext(const GMatrix& ctm) override{
        inverse.setConcat(ctm, local);
        return inverse.invert(&inverse);
        
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint start = inverse.mapXY(x + 0.5f, y + 0.5f);
        float a = inverse[0], d = inverse[3];
        for(int i = 0; i < count; i++){
            int srcX = GFloorToInt(start.fX);
            int srcY = GFloorToInt(start.fY);
             if (tile == TileMode::kRepeat) {
                srcX %= bitmap.width();
                if (srcX < 0) {
                    srcX += bitmap.width();
                }

                srcY %= bitmap.height();
                if (srcY < 0) {
                    srcY += bitmap.height();
                }
            } else if (tile == TileMode::kMirror) {
                float x1 = start.fX / bitmap.width();
                float y1 = start.fY / bitmap.height();

                x1 *= .5;
                x1 = x1 - GFloorToInt(x1);
                if (x1 > .5) {
                    x1 = 1 - x1;
                }
                x1 *= 2;

                y1 *= .5;
                y1 = y1 - GFloorToInt(y1);
                if (y1 > .5) {
                    y1 = 1 - y1;
                }
                y1 *= 2;
                srcX = GFloorToInt(x1 * bitmap.width());
                srcY = GFloorToInt(y1 * bitmap.height());
            }
            else{
                srcX = std::max(0, std::min(bitmap.width() - 1, srcX));
                srcY = std::max(0, std::min(bitmap.height() - 1, srcY));
            }
            row[i] = *bitmap.getAddr(srcX, srcY);
            start.fX += a;
            start.fY += d;
        }
        
        
    }
private:
    GBitmap bitmap;
    GMatrix inverse;
    GMatrix local;
    TileMode tile;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tile){
    if(!bitmap.pixels())
        return nullptr;
    return std::unique_ptr<GShader>(new MyBitmapShader(bitmap, localMatrix, tile));
}