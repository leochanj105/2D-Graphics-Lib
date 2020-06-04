#include "include/GMatrix.h"
#include "include/GPath.h"
#include "include/GPoint.h"
#include "include/GRect.h"
#include <math.h>


GPath& GPath::addRect(const GRect& r, Direction dir){
    this -> moveTo({r.left(), r.top()});
    if(dir == Direction::kCW_Direction){
        this -> lineTo({r.right(), r.top()});
        this -> lineTo({r.right(), r.bottom()});
        this -> lineTo({r.left(), r.bottom()});
    }
    else{
        this -> lineTo({r.left(), r.bottom()});
        this -> lineTo({r.right(), r.bottom()});
        this -> lineTo({r.right(), r.top()});
    }
    return *this;
}
GPath& GPath::addPolygon(const GPoint pts[], int count) {
    if(count < 1) return *this;
    this -> moveTo(pts[0]);
    for(int i = 1; i < count; i++){
        this -> lineTo(pts[i]);
    }
    return *this;
}

GRect GPath::bounds() const{
    int count = this -> fPts.size();
    if(count == 0) return GRect::MakeWH(0,0);
    float maxx = fPts[0].fX, maxy = fPts[0].fY, minx = fPts[0].fX, miny = fPts[0].fY;
    for(int i = 1; i < count; i++){
        if(fPts[i].fX < minx) 
            minx = fPts[i].fX;
        else if(fPts[i].fX > maxx)
            maxx = fPts[i].fX;

        if(fPts[i].fY < miny) 
            miny = fPts[i].fY;
        else if(fPts[i].fY > maxy)
            maxy = fPts[i].fY; 
    }
    return GRect::MakeLTRB(minx, miny, maxx, maxy);
}

void GPath::transform(const GMatrix& m){
    m.mapPoints(fPts.data(), fPts.data(), fPts.size());
}

GPath& GPath::addCircle(GPoint center, float radius, Direction direction){
    GMatrix m;
    GPoint p1, p2 = {radius, 0};
    this -> moveTo(center + p2);
    float tan8 = (float) tan(M_PI / 8);
    if(direction == Direction::kCW_Direction){
        m.setRotate(M_PI / 4);
        p1 = {radius, radius * tan8};
    } else{
        m.setRotate(-M_PI / 4);
        p1 = {radius, -radius * tan8};
    }

    p2 = m.mapPt(p2);
    for(int i = 0; i < 8; i++){
        this -> quadTo(center + p1, center + p2);
        p1 = m.mapPt(p1);
        p2 = m.mapPt(p2);

    }
    return *this;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    float tc = 1 - t;
    dst[0] = src[0];
    dst[4] = src[2];
    dst[1] = GPoint::Make(tc * src[0].fX + t * src[1].fX, tc * src[0].fY + t * src[1].fY);
    dst[3] = GPoint::Make(tc * src[1].fX + t * src[2].fX, tc * src[1].fY + t * src[2].fY);
    dst[2] = GPoint::Make(tc * dst[1].fX + t * dst[3].fX, tc * dst[1].fY + t * dst[3].fY);
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
    float tc = 1 - t;
    dst[0] = src[0];
    dst[6] = src[3];
    dst[1] = GPoint::Make(tc * src[0].fX + t * src[1].fX, tc * src[0].fY + t * src[1].fY);
    dst[5] = GPoint::Make(tc * src[2].fX + t * src[3].fX, tc * src[2].fY + t * src[3].fY);
    GPoint mid = GPoint::Make(tc * src[1].fX + t * src[2].fX, tc * src[1].fY + t * src[2].fY);
    dst[2] = GPoint::Make(tc * dst[1].fX + t * mid.fX, tc * dst[1].fY + t * mid.fY);
    dst[4] = GPoint::Make(tc * mid.fX + t * dst[5].fX, tc * mid.fY + t * dst[5].fY);
    dst[3] = GPoint::Make(tc * dst[2].fX + t * dst[4].fX, tc * dst[2].fY + t * dst[4].fY);
}
