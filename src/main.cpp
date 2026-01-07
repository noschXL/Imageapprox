#include <algorithm>
#include <array>
#include <cstdlib>
#include <time.h>
#include "../include/Window.hpp"
#include <filesystem>
#include <iostream>

constexpr int NUM_RECTS_PER_ITERATION = 100;
constexpr int MAX_ITERATIONS = 3000;

constexpr int MAX_START_SIZE = 300;
constexpr int MIN_END_SIZE = 25;

struct ColorRect {
  Rectangle rec;
  Color c;
};

Color GetBestRectColor(Rectangle rec, Image original) {
  long long rsum = 0, gsum = 0, bsum = 0, pixelcount = 0;

  for (int x = rec.x; x < rec.x + rec.width; x++) {
    for (int y = rec.y; y < rec.y + rec.height; y++) {
      Color px = GetImageColor(original, x, y);
      rsum += px.r;
      gsum += px.g;
      bsum += px.b;
      pixelcount++;
    }
  }
  if (pixelcount == 0) {
    return Color{0, 0, 0, 255};
  }

  Color c;
  c.r = (unsigned char)(rsum / pixelcount);
  c.g = (unsigned char)(gsum / pixelcount);
  c.b = (unsigned char)(bsum / pixelcount);
  c.a = 255;

  return c;

}

int lerp(int a, int b, float t) {
    return (int)(a + (b - a) * t);
}

ColorRect GenerateRandomRect(int w,int h, Image original, float iteration) {

  float delta = iteration / MAX_ITERATIONS;
  delta = delta*delta;

  int maxSize = MAX_START_SIZE * powf((float)MIN_END_SIZE / MAX_START_SIZE, delta);
  maxSize = std::clamp(maxSize, 1, MAX_START_SIZE);

  Rectangle rec;

  rec.x = rand() % w;
  rec.y = rand() % h;

  rec.width = rand() % int(w-rec.x) % maxSize;
  rec.height = rand() % int(h-rec.y) % maxSize;

  ColorRect crect;
  crect.rec=rec;
  crect.c = GetBestRectColor(rec, original);

  return crect;
}
int RectangleDeltaError(ColorRect rect, Image current, Image original) {
  int delta = 0;

  for (int x = rect.rec.x; x < rect.rec.x + rect.rec.width; x++) {
    for (int y = rect.rec.y; y < rect.rec.y + rect.rec.height; y++) {

      Color cur = GetImageColor(current, x, y);
      Color org = GetImageColor(original, x, y);

      int before =
        (cur.r - org.r) * (cur.r - org.r) +
        (cur.g - org.g) * (cur.g - org.g) +
        (cur.b - org.b) * (cur.b - org.b);

      int after =
        (rect.c.r - org.r) * (rect.c.r - org.r) +
        (rect.c.g - org.g) * (rect.c.g - org.g) +
        (rect.c.b - org.b) * (rect.c.b - org.b);

      delta += before - after;
    }
  }
  return delta;
}

int RectangleError(ColorRect rect, Image current) {
  int e = 0;
  for (int x = rect.rec.x; x < rect.rec.x + rect.rec.width;x++) {
    for (int y = rect.rec.y; y < rect.rec.y + rect.rec.height;y++) {
      Color px = GetImageColor(current, x, y);
      e += (px.r - rect.c.r) * (px.r - rect.c.r);
      e += (px.g - rect.c.g) * (px.g - rect.c.g);
      e += (px.b - rect.c.b) * (px.b - rect.c.b);
    }
  }

  return e;

}

void DebugColorRect (ColorRect rec) {
  std::cout << "x: " << rec.rec.x
            << "y: " << rec.rec.y
            << "w: " << rec.rec.width
            << "h: " << rec.rec.height
            << "r: " << rec.c.r
            << "g: " << rec.c.g
            << "b: " << rec.c.b
            << "\n";
}

int main() {
  std::cout << "CWD: " << std::filesystem::current_path() << "\n";
  srand(time(0));
  int screenWidth = 1980;
  int screenHeight = 1080;

  raylib::Window window(screenWidth, screenHeight, "raylib-cpp - basic window");

  window.SetFullscreen(true);

  Image orgImg = LoadImage("input.png");

  int w = orgImg.width / 4;
  int h = orgImg.height / 4;

  ImageResize(&orgImg, w, h);

  RenderTexture2D currentTex = LoadRenderTexture(w, h);
  SetTargetFPS(60);

  Image currentImg = LoadImageFromTexture(currentTex.texture);
  int iteration = 0;


  while (!window.ShouldClose()) {
    currentImg = LoadImageFromTexture(currentTex.texture);

    int besterror = 0;
    int bestrect = 0;
    std::array<ColorRect, NUM_RECTS_PER_ITERATION> rects;
    for (int i = 0; i < NUM_RECTS_PER_ITERATION; i++) {
      rects[i] = GenerateRandomRect(w, h, orgImg, (float)iteration);
      int d = RectangleDeltaError(rects[i], currentImg, orgImg);
      if (d > besterror) {
        besterror = d;
        bestrect = i;
      }
    }

    BeginTextureMode(currentTex);
    DrawRectangleRec(rects[bestrect].rec, rects[bestrect].c);
    EndTextureMode();

    DebugColorRect(rects[bestrect]);


    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(currentTex.texture, 0, 0, WHITE);
    EndDrawing();

    ImageDrawRectangleRec(&currentImg, rects[bestrect].rec, rects[bestrect].c);

    iteration++;
    std::cout << iteration << "\n";
  }



  return 0;
}
