#pragma once

// inlcude this before MyApp
// #inlcude "Function-From-Max.hpp"

struct MyApp;
class FunctionFromMax {
  MyApp* app;
  al::Mesh circle;
  al::Mesh line;
  std::vector<al::Vec3f> point;
  std::vector<bool> hover;

  //
  // Helper Methods
  //

  al::Vec3d unproject(al::Vec3d screenPos);
  al::Rayd getPickRay(int screenX, int screenY);

 public:
  FunctionFromMax(MyApp* that);

  // set of pairs or (value,time) where time and value are normalized on (0, 1)
  std::vector<al::Vec2f> points();

  void onCreate();
  void onDraw(al::Graphics& g);
  bool onMouseDrag(const al::Mouse& m);
  bool onMouseDown(const al::Mouse& m);
  bool onMouseUp(const al::Mouse& m);
  bool onMouseMove(const al::Mouse& m);
};

