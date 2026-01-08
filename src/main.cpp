#include <algorithm>
#include <array>
#include <cstdlib>
#include <time.h>
#include "../include/Window.hpp"
#include <filesystem>
#include <iostream>

constexpr int NUM_RECTS_PER_ITERATION = 100;
constexpr int MAX_ITERATIONS = 10000;

constexpr int MAX_START_SIZE = 200;
constexpr int MIN_END_SIZE = 1;

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
  if (pixelcount == 0) {
    std::cout << "pixelcount is 0\n";
  }
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
  delta = delta*delta*delta;

  int maxSize = MAX_START_SIZE * powf((float)MIN_END_SIZE / MAX_START_SIZE, delta);
  maxSize = std::clamp(maxSize, MIN_END_SIZE+1, MAX_START_SIZE);

  Rectangle rec;

  rec.x = rand() % (w-MIN_END_SIZE);
  rec.y = rand() % (h-MIN_END_SIZE);

  rec.width = rand() % std::min(int(w-rec.x - MIN_END_SIZE), maxSize - MIN_END_SIZE) + MIN_END_SIZE;
  rec.height = rand() % std::min(int(h-rec.y - MIN_END_SIZE), maxSize - MIN_END_SIZE) + MIN_END_SIZE;

  if (rec.width == 0 || rec.height == 0) {
    std::cout << "heheheha\n";
  }

  ColorRect crect;
  crect.rec=rec;
  crect.c = GetBestRectColor(rec, original);
  
  if (rec.width == 0 || rec.height == 0) {
    std::cout << "heheheha\n";
    crect = GenerateRandomRect(w, h, original, iteration);
  }

  return crect;
}

void ColorDebug(Color col) {
  std::cout << "r: " << (int)col.r << " g: " << (int)col.g << " b: " << (int)col.b << "\n";
}

float RectangleDeltaError(ColorRect rect, Image current, Image original, bool debug = false) {
  int delta = 0;
  float aera = rect.rec.width * rect.rec.height;

  for (int x = rect.rec.x; x < rect.rec.x + rect.rec.width; x++) {
    for (int y = rect.rec.y; y < rect.rec.y + rect.rec.height; y++) {

      Color cur = GetImageColor(current, x, y);
      if (debug) {
        ColorDebug(cur);
      }
      Color org = GetImageColor(original, x, y);

      int before =
        (cur.r - org.r) * (cur.r - org.r) +
        (cur.g - org.g) * (cur.g - org.g )+
        (cur.b - org.b) * (cur.b - org.b);

      int after =
        (rect.c.r - org.r) * (rect.c.r - org.r) +
        (rect.c.g - org.g) * (rect.c.g - org.g) +
        (rect.c.b - org.b) * (rect.c.b - org.b);

      delta += before - after;
    }
  }
  return (float)delta - aera;
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
            << " y: " << rec.rec.y
            << " w: " << rec.rec.width
            << " h: " << rec.rec.height
            << " r: " << (int)rec.c.r
            << " g: " << (int)rec.c.g
            << " b: " << (int)rec.c.b
            << "\n";
}

int main() {
  std::cout << "CWD: " << std::filesystem::current_path() << "\n";
  srand(time(0));
  int screenWidth = 1366;
  int screenHeight = 768;

  raylib::Window window(screenWidth, screenHeight, "raylib-cpp - basic window");

  window.SetFullscreen(true);

  Image orgImg = LoadImage("input.png");

  int w = orgImg.width / 1.5;
  int h = orgImg.height / 1.5;

  ImageResize(&orgImg, w, h);

  RenderTexture2D currentTex = LoadRenderTexture(w, h);
  SetTargetFPS(60);

  Image currentImg = LoadImageFromTexture(currentTex.texture);
  int iteration = 0;


  while (!window.ShouldClose()) {
    if (iteration >= MAX_ITERATIONS) {
      BeginDrawing();
      ClearBackground(BLACK);
      DrawTexture(currentTex.texture, 0, 0, WHITE);
      EndDrawing();
      continue;
    }
    float besterror = 1e-9f;
    int bestrect = 0;
    std::array<ColorRect, NUM_RECTS_PER_ITERATION> rects;
    for (int i = 0; i < NUM_RECTS_PER_ITERATION; i++) {
      float d = 0;
      try {
        rects[i] = GenerateRandomRect(w, h, orgImg, (float)iteration);
        d = RectangleDeltaError(rects[i], currentImg, orgImg);
      } catch (int err) {
        std::cout << "pranked, SIG: " << err << "\n";
      }
      if (d > besterror) {
        besterror = d;
        bestrect = i;
      }
    }
    std::cout << besterror << "\n";
    
    ImageDrawRectangleRec(&currentImg, rects[bestrect].rec, rects[bestrect].c);
    UpdateTexture(currentTex.texture, currentImg.data);


    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(currentTex.texture, 0, 0, WHITE);
    EndDrawing();


    iteration++;
    std::cout << iteration << "\n";
  }



  return 0;
}
