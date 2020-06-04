
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "GBlend.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"

class MyLinearGradientShader: public GShader{

public:
    MyLinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, TileMode tile){
        this->colors = (GColor*) malloc(count * sizeof(GColor));
        this -> tile = tile;
        memcpy(this->colors, colors, count * sizeof(GColor));
        this->colorCount = count;
        if(p0.fX > p1.fX)std::swap(p0, p1);
        float dx = p1.fX - p0.fX, dy = p1.fY - p0.fY;
        lmat.set6(dx, -dy, p0.fX, dy, dx, p0.fY);
    }

    bool isOpaque() override{
        return false;
    }


    bool setContext(const GMatrix& ctm) override{
        inverse.setConcat(ctm, lmat);
        return inverse.invert(&inverse);    
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint start = inverse.mapXY(x, y);
        float a = inverse[0];
        float ft;
        for(int i = 0; i < count; i++){
            ft = start.fX;
            if (tile == TileMode::kRepeat) {
                ft = ft - GFloorToInt(ft);
            } else if (tile == TileMode::kMirror) {
                ft *= 0.5;
                ft = ft - GFloorToInt(ft);
                if (ft > .5) {
                    ft = 1 - ft;
                }
                ft *= 2;
            }
            else{
                ft = std::max(0.0f, std::min(1.0f, ft));
            }
            ft = ft * (colorCount - 1);
            int idx = GFloorToInt(ft);
            float loc = ft - idx ;
            GColor c1 = colors[idx].pinToUnit(), c2 = colors[idx + 1].pinToUnit();
            row[i] = GColorToPixel(GColor::MakeARGB(c1.fA * (1 - loc) + c2.fA * loc,
                                                    c1.fR * (1 - loc) + c2.fR * loc,
                                                    c1.fG * (1 - loc) + c2.fG * loc,
                                                    c1.fB * (1 - loc) + c2.fB * loc));
           
            start.fX += a;
        }
        
    }
private:
    GColor* colors;
    GMatrix inverse;
    GMatrix lmat;
    int colorCount;
    TileMode tile;
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tile){
    if(count < 1) return nullptr;
    return std::unique_ptr<GShader>(new MyLinearGradientShader(p0, p1, colors, count, tile));
}