#include <array>
#include <cstdlib>
#include <time.h>
#include "../../include/Window.hpp"
#include <filesystem>
#include <iostream>

constexpr int NUM_RECTS_PER_ITERATION = 100;

struct ColorRect {
  Rectangle rec;
  Color c;
};

Color GetBestRectColor(Rectangle rec, Image original) {
  int rsum,gsum,bsum, pixelcount;
  for (int x = rec.x; x < rec.width+1;x++) {
    for (int y = rec.y; y < rec.height+1;y++) {
      Color px = GetImageColor(original, x, y);
      rsum += px.r;
      gsum += px.g;
      bsum += px.b;
      pixelcount++;
    }
  }

  Color c;
  c.r = rsum / pixelcount;
  c.g = gsum / pixelcount;
  c.b = bsum / pixelcount;
  c.a = 255;

  return c;

}

ColorRect GenerateRandomRect(int w,int h, Image original) {
  Rectangle rec;

  rec.x = rand() % (w+1);
  rec.y = rand() % (h+1);

  rec.width = rand() % int(w-rec.x+1);
  rec.height = rand() % int(h-rec.y+1);

  ColorRect crect;
  crect.rec=rec;
  crect.c=GetBestRectColor(rec, original);

  return crect;
}

int RectangleError(ColorRect rect, Image current) {
  int e;
  for (int x = rect.rec.x; x < rect.rec.width+1;x++) {
    for (int y = rect.rec.y; y < rect.rec.height+1;y++) {
      Color px = GetImageColor(current, x, y);
      e += (px.r - rect.c.r) * (px.r - rect.c.r);
      e += (px.g - rect.c.g) * (px.g - rect.c.g);
      e += (px.b - rect.c.b) * (px.b - rect.c.b);
    }
  }

  return e;

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


  RenderTexture2D currentTex;
  SetTargetFPS(60);
  
  while (!window.ShouldClose()) {
    Image currentImg = LoadImageFromTexture(currentTex.texture);

    int besterror = ~0;
    int bestrect = 0;
    std::array<ColorRect, NUM_RECTS_PER_ITERATION> rects;
    for (int i = 0; i < NUM_RECTS_PER_ITERATION; i++) {
      rects[i] = GenerateRandomRect(w, h, orgImg);
      int e = RectangleError(rects[i], currentImg);
      if (e < besterror) {
        besterror = e;
        bestrect = i;
      }
    }

    BeginTextureMode(currentTex);
    DrawRectangleRec(rects[bestrect].rec, rects[bestrect].c);
    EndTextureMode();


    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(currentTex.texture, 0, 0, WHITE);
    EndDrawing();
  }



  return 0;
}
