#include <math.h>

#include "include/GMatrix.h"
#include "include/GPoint.h"

void GMatrix::setIdentity(){
    this -> set6(1,0,0,0,1,0);
}

void GMatrix::setTranslate(float tx, float ty){
    this -> set6(1,0,tx, 0, 1, ty);
}

void GMatrix::setScale(float sx, float sy){
    this -> set6(sx, 0, 0, 0, sy, 0);
}

void GMatrix::setRotate(float radians){
    this -> set6(cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0);
}
void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo){
    float a11 = primo[0] * secundo[0] + primo[3] * secundo[1];
    float a12 = primo[1] * secundo[0] + primo[4] * secundo[1];
    float a13 = primo[2] * secundo[0] + primo[5] * secundo[1] + secundo[2];
    float a21 = primo[0] * secundo[3] + primo[3] * secundo[4];
    float a22 = primo[1] * secundo[3] + primo[4] * secundo[4];
    float a23 = primo[2] * secundo[3] + primo[5] * secundo[4] + secundo[5];

    this -> set6(a11, a12, a13, a21, a22, a23);
}

bool GMatrix::invert(GMatrix* inverse) const{
    float a11 = fMat[0], a12 = fMat[1], a13 = fMat[2], a21 = fMat[3], a22 = fMat[4], a23 = fMat[5];
    float det = a11 * a22 - a12 * a21;
    float inv = 1 / det;
    if(det == 0.0) return false;
    inverse -> set6(a22 * inv, -a12 * inv, -(a13 * a22 - a12 * a23) * inv,
                    -a21 * inv, a11 * inv, (a13 * a21 - a11 * a23) * inv);
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const{
    for(int i = 0; i < count; i++){
        GPoint point = src[i];
        float x = point.x(), y = point.y();
        dst[i] = GPoint::Make(fMat[0] * x + fMat[1] * y + fMat[2],
                              fMat[4] * y + fMat[3] * x + fMat[5]);
    }
}