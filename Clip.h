
#ifndef CLIP_H
#define CLIP_H
#include "include/GRect.h"
#include "include/GPoint.h"
#include "include/GMath.h"
#include <vector>




struct GEdge{
      int top;
      int bottom;
      float slope;
      float currentX;
      int wind;
      GEdge(GPoint, GPoint, int wind);
      GEdge(){}
      bool operator<(const GEdge& other);

};

void clip(GPoint p0, GPoint p1, GRect bounds, std::vector<GEdge> &edges);


#endif