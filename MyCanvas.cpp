
#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include "include/GPaint.h"
#include "include/GRect.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPath.h"
#include "TriShader.h"
#include "ProxyShader.h"
#include "CompositeShader.h"
#include "Clip.h"
#include "GBlend.h"
#include <vector>
#include <stack>

class MyCanvas : public GCanvas {
  public:
    MyCanvas(const GBitmap& device): fDevice(device){
      GMatrix mat;
      mat.setIdentity();
      matStack.push(mat);
    }

    void save() override{
      GMatrix ctm = matStack.top(); 
      GMatrix copy(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
      matStack.push(copy);
    }

    void restore() override{
      matStack.pop();
    }
    void concat(const GMatrix& matrix) override{
      matStack.top().preConcat(matrix);
    }
    void drawPath(const GPath& path, const GPaint& paint){
        GPath::Edger edger = GPath::Edger(path);
        GPath::Verb verb;
        std::vector<GEdge> edges;
        GRect bound = GRect::MakeWH(fDevice.width(), fDevice.height());
        GPoint points[4];
        float d, t, dt;
        int N;
        GVector dist;
        while(true) {
            verb = edger.next(points);
            if (verb == GPath::Verb::kLine){
              matStack.top().mapPoints(points, points, 2);
              clip(points[0], points[1], bound, edges);
            }
            else if(verb == GPath::Verb::kQuad){
              matStack.top().mapPoints(points, points, 3);
              d = ((points[0] - points[1]) + (points[2] - points[1])).length();
              N = GCeilToInt(sqrt(d));
              dt = 1.0 / N;
              for(int i = 0; i < N; i++){
                clip(pointAtQuad(points[0], points[1], points[2], dt * i), 
                     pointAtQuad(points[0], points[1], points[2], dt * (i + 1)), bound, edges);
              }
            } 
            else if(verb == GPath::Verb::kCubic){
              matStack.top().mapPoints(points, points, 4);
              d = std::max(((points[0] - points[1]) + (points[2] - points[1])).length(),
                           ((points[1] - points[2]) + (points[3] - points[2])).length());
              N = GCeilToInt(sqrt(d * 3.0));
              dt = 1.0 / N;
              for(int i = 0; i < N; i++){
                clip(pointAtCubic(points[0], points[1], points[2], points[3], dt * i), 
                     pointAtCubic(points[0], points[1], points[2], points[3], dt * (i + 1)), bound, edges);
              }
            }
            else if(verb == GPath::Verb::kDone){
              break;
            }

        }
        GScanPath(edges, paint);     
    }

    GPoint pointAtQuad(const GPoint& p1, const GPoint& p2, const GPoint& p3, float t){
      float tc = 1 - t;
      GPoint pa = GPoint::Make(tc * p1.fX + t * p2.fX, tc * p1.fY + t * p2.fY);
      GPoint pb = GPoint::Make(tc * p2.fX + t * p3.fX, tc * p2.fY + t * p3.fY);
      return GPoint::Make(tc * pa.fX + t * pb.fX, tc * pa.fY + t * pb.fY);
    }
    GPoint pointAtCubic(const GPoint& p1, const GPoint& p2, const GPoint& p3, const GPoint& p4, float t){
      float tc = 1 - t;
      GPoint pa = GPoint::Make(tc * p1.fX + t * p2.fX, tc * p1.fY + t * p2.fY);
      GPoint pb = GPoint::Make(tc * p2.fX + t * p3.fX, tc * p2.fY + t * p3.fY);
      GPoint pc = GPoint::Make(tc * p3.fX + t * p4.fX, tc * p3.fY + t * p4.fY);
      GPoint pd = GPoint::Make(tc * pa.fX + t * pb.fX, tc * pa.fY + t * pb.fY);
      GPoint pe = GPoint::Make(tc * pb.fX + t * pc.fX, tc * pb.fY + t * pc.fY);
      return GPoint::Make(tc * pd.fX + t * pe.fX, tc * pd.fY + t * pe.fY);
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint& paint){
      // printf("count = %d, len of idx = %d\n", count, (sizeof(indices) / sizeof(indices[0])));
      int n = 0;
      GPoint p0, p1, p2;
      GColor c1, c2, c3;
      for(int i = 0; i < count; i++){
        p0 = verts[indices[n]];
        p1 = verts[indices[n + 1]];
        p2 = verts[indices[n + 2]];
        GPoint points[] = {p0, p1, p2};
        GColor *cols = nullptr;
        GPoint *textures = nullptr;
        if(colors){
          cols = (GColor*) malloc(sizeof(GColor) * 3);
          cols[0] = colors[indices[n]];
          cols[1] = colors[indices[n + 1]];
          cols[2] = colors[indices[n + 2]];
        }
        if(texs){
          // printf("%d\n", count);
          textures = (GPoint*) malloc(sizeof(GPoint) * 3);
          textures[0] = texs[indices[n]];
          textures[1] = texs[indices[n + 1]];
          textures[2] = texs[indices[n + 2]];
        }
        drawTriangle(points, cols, textures, paint.getShader());
        n += 3;
      }
    }


    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint& paint){
      float factor = 1.0 / level;
      int count = 2 * level * level, idx = 0;
      int indices[count * 3];
      GPoint points[(level + 1) * (level + 1)];
      GPoint p03 = (verts[3] - verts[0]) * factor, p12 = (verts[2] - verts[1]) * factor;
      // printf("factor = %2.2f\n", factor);
      // printf("v0 = (%2.2f, %2.2f), v3 = (%2.2f, %2.2f)\n", verts[0].fX, verts[0].fY, verts[3].fX, verts[3].fY);
      // printf("p03 = (%2.2f, %2.2f), p12 = (%2.2f, %2.2f)\n\n", p03.fX, p03.fY, p12.fX, p12.fY);
      GPoint *textures = nullptr;
      GColor *cols = nullptr;

      GPoint left, right;
      
      left = verts[0];
      right = verts[1];
      for(int i = 0; i <= level; i++){
        GPoint pos = (right - left) * factor;
        for(int j = 0; j <= level; j++){
          points[i * (level + 1) + j] = left + pos * j;
        }
        left = left + p03;
        right = right + p12;
      }

      int pos = 0, i0, i1, i2, i3;
      for(int i = 0; i < level; i++){
        for(int j = 0; j < level; j++){
          i0 = i * (level + 1) + j;
          i1 = i0 + 1;
          i2 = i1 + level + 1;
          i3 = i2 - 1;
          indices[pos] = i0;
          indices[pos + 1] = i1;
          indices[pos + 2] = i3;
          indices[pos + 3] = i1;
          indices[pos + 4] = i3;
          indices[pos + 5] = i2;
          pos += 6;
        }
      }
      if(texs){
        textures = (GPoint*) malloc(sizeof(GPoint) * (level + 1) * (level + 1));
        p03 = (texs[3] - texs[0]) * factor;
        p12 = (texs[2] - texs[1]) * factor;
        left = texs[0];
        right = texs[1];
        for(int i = 0; i <= level; i++){
          GPoint pos = (right - left) * factor;
          for(int j = 0; j <= level; j++){
            textures[i * (level + 1) + j] = left + pos * j;
          }
          left = left + p03;
          right = right + p12;
        }
      }
      if(colors){
        cols = (GColor*) malloc(sizeof(GColor) * (level + 1) * (level + 1));
        GColor c03 = colors[0], c12 = colors[1];
        float a03 = (colors[3].fA - colors[0].fA) * factor, r03 = (colors[3].fR - colors[0].fR) * factor, 
              g03 = (colors[3].fG - colors[0].fG) * factor, b03 = (colors[3].fB - colors[0].fB) * factor;
        float a12 = (colors[2].fA - colors[1].fA) * factor, r12 = (colors[2].fR - colors[1].fR) * factor, 
              g12 = (colors[2].fG - colors[1].fG) * factor, b12 = (colors[2].fB - colors[1].fB) * factor;
        float alr, rlr, glr, blr;
        GColor cl = colors[0], cr = colors[1];
        for(int i = 0; i <= level; i++){
          alr = (cr.fA - cl.fA) * factor;
          rlr = (cr.fR - cl.fR) * factor;
          glr = (cr.fG - cl.fG) * factor;
          blr = (cr.fB - cl.fB) * factor;
          for(int j = 0; j <= level; j++){
            cols[i * (level + 1) + j] = GColor::MakeARGB(cl.fA + alr * j, cl.fR + rlr * j,
                                                           cl.fG + glr * j, cl.fB + blr * j);
          }
          cl.fA += a03;
          cl.fR += r03;
          cl.fG += g03;
          cl.fB += b03;
          cr.fA += a12;
          cr.fR += r12;
          cr.fG += g12;
          cr.fB += b12;
        }
      }
      drawMesh(points, cols, textures, count, indices, paint);
      
    }

    void drawTriangle(const GPoint pts[3], const GColor colors[3], const GPoint tex[3], GShader* originalShader) {
      TriShader tri(pts, colors);
      ProxyShader proxy(originalShader, pts, tex);
      CompositeShader comp(&tri, &proxy);
      GShader *s;
      if(colors)  s= &tri;
      if(tex && !colors) s = &proxy;
      if(colors && tex) s = &comp;
      GPaint paint(s);
      drawConvexPolygon(pts, 3, s);
    }

    void drawPaint(const GPaint& paint) override{
      GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
      drawRect(bounds, paint);
    }

     void drawRect(const GRect& rect, const GPaint& paint) override{
          GPoint points[4] = {
            GPoint::Make(rect.left(), rect.top()),
            GPoint::Make(rect.right(), rect.top()),
            GPoint::Make(rect.right(), rect.bottom()),
            GPoint::Make(rect.left(), rect.bottom())
          };
          drawConvexPolygon(points, 4, paint);
     }

    void GBlitRow(int y, int left, int right, const GPaint& paint){
       left = std::max(0, left);
       right = std::min(fDevice.width(), right);
       GPixel* row = fDevice.getAddr(0, y);
       GShader* shader = paint.getShader();
       int count = right - left;
       if(shader != nullptr)
         if(!shader-> setContext(matStack.top())) return;
       GShade(paint, row + left, left, y, count);
     }


     static bool compareX(GEdge e1, GEdge e2){
       return e1.currentX < e2.currentX;
     }

     void GScanPath(std::vector<GEdge> edges, const GPaint& paint){
       int count = edges.size();
       if(count < 2)  return;
       std::sort(edges.begin(), edges.end());
       int y = edges[0].top;
       int end = 0;
       while(count > 0){
          while(edges[end].top <= y && end < count) end++;
          std::sort(edges.begin(), edges.begin() + end, compareX);
          int wind = 0, x0 = 0, x1 = 0, previous = 0;
          for(int idx = 0; idx < end; idx++){
            previous = wind;
            wind += edges[idx].wind;
            if(previous == 0 && wind != 0) 
              x0 = idx;
            else if(previous != 0 && wind == 0){
              x1 = idx;
              int l = GRoundToInt(edges[x0].currentX), r = GRoundToInt(edges[x1].currentX);
              if(r > l) GBlitRow(y, l,  r, paint);
            }
          }
          
          y++;
          int i = 0;
          while(i < end){
            if(y >= edges[i].bottom) {
              edges.erase(edges.begin() + i);
              end--;
              count--;
            }
            else{
              edges[i].currentX += edges[i].slope;
              i++;
            }
          }
       }
     }

     void GScan(std::vector<GEdge> edges, const GPaint& paint){
       if(edges.size() < 2)  return;
       std::sort(edges.begin(), edges.end());
       GEdge left = edges[0], right = edges[1];
       int next = 2, y = edges[0].top;

       while(y < edges.back().bottom){
          int l = GRoundToInt(left.currentX), r = GRoundToInt(right.currentX);
          if(r > l) GBlitRow(y, l, r, paint);
          left.currentX += left.slope;
          right.currentX += right.slope;
          y++;
          if(y >= left.bottom) {
            left = edges[next++];
            if(left.currentX > right.currentX) std::swap(left, right);  
          }
          if(y >= right.bottom){
            right = edges[next++];
            if(left.currentX > right.currentX) std::swap(left, right);  
          }
          
       }
     }
     void drawConvexPolygon(const GPoint srcPoints[], int count, const GPaint& paint) override{
       if(count < 3) return;
       GPoint points[count];
       matStack.top().mapPoints(points, srcPoints, count);
       std::vector<GEdge> edges;
       GRect bound = GRect::MakeWH(fDevice.width(), fDevice.height());
       for(int i = 0; i < count - 1; i++)
          clip(points[i], points[i + 1], bound, edges);
       clip(points[count - 1], points[0], bound, edges);

       GScan(edges, paint);
     }


  private:
    const GBitmap fDevice;
    std::stack<GMatrix> matStack;
};


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device){
  if(!device.pixels()){
    return nullptr;
  }
  return std::unique_ptr<GCanvas>(new MyCanvas(device));
}
