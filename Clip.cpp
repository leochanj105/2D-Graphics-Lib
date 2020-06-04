#include "Clip.h"

GEdge::GEdge(GPoint p0, GPoint p1, int wind){

    if(p0.y() > p1.y()){
        std::swap(p0, p1);
        wind = -wind;
    }
    // if(p0.y() > p1.y()){
    //     top = GRoundToInt(p1.y());
    //     bottom = GRoundToInt(p0.y());
    //     slope = (p0.x() - p1.x()) / (p0.y() - p1.y());
    //     currentX = p1.x() + slope * ((float)top - p1.y() + 0.5f);
    // } else{ 
    top = GRoundToInt(p0.y());
    bottom = GRoundToInt(p1.y());
    slope = (p1.x() - p0.x()) / (p1.y() - p0.y());
    currentX = p0.x() + slope * ((float)top - p0.y() + 0.5f);
        // wind = -wind;
    // }
    this -> wind = wind;
}


bool GEdge::operator<(const GEdge& other){
    if(this->top == other.top){
        if(this->currentX == other.currentX)
            return this->slope < other.slope;
        else
            return this->currentX < other.currentX;
    }
    return this->top < other.top;
}
bool checkHorizontal(const GPoint& pa, const GPoint& pb){
    return GRoundToInt(pa.fY) == GRoundToInt(pb.fY);
}

void clip(GPoint p0, GPoint p1, GRect bounds, std::vector<GEdge> &edges){
    
        int wind = 1;
        if(p0.y() > p1.y()){
            std::swap(p0, p1);
            wind = -wind;
        }
        if(p1.y() < bounds.top() || p0.y() >= bounds.bottom()) return;
        if(p0.y() < bounds.top()){
            float x_clipped = p0.x() + (p1.x() - p0.x()) * (bounds.top() - p0.y()) / (p1.y() - p0.y());
            p0.set(x_clipped, bounds.top());
        }

        if(p1.y() > bounds.bottom()){
            float x_clipped = p1.x() - (p1.x() - p0.x()) * (p1.y() - bounds.bottom()) / (p1.y() - p0.y());
            p1.set(x_clipped, bounds.bottom());
        }
        
        if(p0.x() > p1.x()){
            std::swap(p0, p1);
            wind = -wind;
        };

        if(p1.x() <= bounds.left()){
            p0.fX = p1.fX = bounds.left();
            if(!checkHorizontal(p0,p1))
                edges.push_back(GEdge(p0, p1, wind));
            return;
        }

        if(p0.x() >= bounds.right()){
            p0.fX = p1.fX = bounds.right();
            if(!checkHorizontal(p0,p1))
                edges.push_back(GEdge(p0, p1, wind));
            return;
        }
        if(p0.x() < bounds.left()){
            GVector v = p1 - p0;
            v = v * ((bounds.left() - p0.x()) / (p1.x()- p0.x()));
            if(!checkHorizontal(p0, p0 + v))
                edges.push_back(GEdge(GPoint::Make(bounds.left(), p0.y()), p0 + v, wind));
            p0 = p0 + v;
        }

        if(p1.x() >= bounds.right()){
           GVector v = p0 - p1;
            v = v * ((p1.x() - bounds.right()) / (p1.x()- p0.x()));
            if(!checkHorizontal(p1 + v, p1))
                edges.push_back(GEdge(p1 + v, GPoint::Make(bounds.right(), p1.y()), wind));
            p1 = p1 + v;
        }
        if(!checkHorizontal(p0, p1))    
         edges.push_back(GEdge(p0, p1, wind));
}
