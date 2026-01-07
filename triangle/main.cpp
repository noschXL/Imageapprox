#include <algorithm>
#include <array>
#include <time.h>
#include "../include/Window.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

constexpr int NUM_TRIANGLES = 100;

struct Triangle {
  raylib::Vector2 p0,p1,p2;
  Color color;
};

Triangle GenerateRandomTriangle(Image img){
  Triangle t;
  t.p0.x = float(rand() % (img.width+1));
  t.p0.y = float(rand() % (img.height+1));
  t.p1.x = float(rand() % (img.width+1));
  t.p1.y = float(rand() % (img.height+1));
  t.p2.x = float(rand() % (img.width+1));
  t.p2.y = float(rand() % (img.height+1));

  t.color = WHITE;

  return t;
}

bool PointInTriangle(const raylib::Vector2& p, const Triangle& t)
{
    raylib::Vector2 v0 = t.p2 - t.p0;
    raylib::Vector2 v1 = t.p1 - t.p0;
    raylib::Vector2 v2 = p     - t.p0;

    float d00 = v0.x * v0.x + v0.y * v0.y;
    float d01 = v0.x * v1.x + v0.y * v1.y;
    float d11 = v1.x * v1.x + v1.y * v1.y;
    float d20 = v2.x * v0.x + v2.y * v0.y;
    float d21 = v2.x * v1.x + v2.y * v1.y;

    float denom = d00 * d11 - d01 * d01;
    if (denom == 0.0f) return false; // degenerate triangle

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return (u >= 0.0f) && (v >= 0.0f) && (w >= 0.0f);
}

Color ComputeTriangleAvgColor_CPU(
    const Triangle& t,
    const Image& img
){
    long sumR = 0, sumG = 0, sumB = 0;
    int count = 0;

    int minX = std::max(0, (int)std::min({t.p0.x, t.p1.x, t.p2.x}));
    int maxX = std::min(img.width-1, (int)std::max({t.p0.x, t.p1.x, t.p2.x}));
    int minY = std::max(0, (int)std::min({t.p0.y, t.p1.y, t.p2.y}));
    int maxY = std::min(img.height-1, (int)std::max({t.p0.y, t.p1.y, t.p2.y}));

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            if (PointInTriangle({(float)x,(float)y}, t)) {
                Color c = GetImageColor(img, x, y);
                sumR += c.r;
                sumG += c.g;
                sumB += c.b;
                count++;
            }
        }
    }

    if (count == 0) return WHITE;

    return {
        (unsigned char)(sumR / count),
        (unsigned char)(sumG / count),
        (unsigned char)(sumB / count),
        255
    };
}

Color ComputeTriangleAvgColor(
    const Triangle &t,
    RenderTexture2D &targetTex,   // original image as render texture
    RenderTexture2D &accumTex,    // accumulation buffer (reuse every triangle)
    Shader &avgShader,
    int screenWidth,
    int screenHeight
) {
    int loc_p0        = GetShaderLocation(avgShader, "p0");
    int loc_p1        = GetShaderLocation(avgShader, "p1");
    int loc_p2        = GetShaderLocation(avgShader, "p2");
    int loc_res       = GetShaderLocation(avgShader, "resolution");
    int loc_targetTex = GetShaderLocation(avgShader, "targetTex");

    SetShaderValue(avgShader, loc_p0, &t.p0, SHADER_UNIFORM_VEC2);
    SetShaderValue(avgShader, loc_p1, &t.p1, SHADER_UNIFORM_VEC2);
    SetShaderValue(avgShader, loc_p2, &t.p2, SHADER_UNIFORM_VEC2);

    raylib::Vector2 res = { float(screenWidth), float(screenHeight) };
    SetShaderValue(avgShader, loc_res, &res, SHADER_UNIFORM_VEC2);

    SetShaderValueTexture(avgShader, loc_targetTex, targetTex.texture);

    BeginTextureMode(accumTex);
    BeginShaderMode(avgShader);
    DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
    EndShaderMode();
    EndTextureMode();

    Image img = LoadImageFromTexture(accumTex.texture);
    Color pixel = LoadImageColors(img)[0];
    UnloadImage(img);

    if (pixel.a == 0) return WHITE; //this is bad

    Color avg;
    avg.r = pixel.r / pixel.a;
    avg.g = pixel.g / pixel.a;
    avg.b = pixel.b / pixel.a;
    avg.a = 255;

    return avg;
}

float TriangleError(const Triangle &t, RenderTexture2D &original, Image orgimg) {
    
  

    // compute bounding box
    int minX = std::min({ int(t.p0.x), int(t.p1.x), int(t.p2.x) });
    int maxX = std::max({ int(t.p0.x), int(t.p1.x), int(t.p2.x) });
    int minY = std::min({ int(t.p0.y), int(t.p1.y), int(t.p2.y) });
    int maxY = std::max({ int(t.p0.y), int(t.p1.y), int(t.p2.y) });

    float error = 0.0f;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            raylib::Vector2 p = { float(x), float(y) };

            if (PointInTriangle(p, t)) {
                Color orig = GetImageColor(orgimg, x, y);
                Color tri = t.color;

                float dr = float(orig.r) - float(tri.r);
                float dg = float(orig.g) - float(tri.g);
                float db = float(orig.b) - float(tri.b);

                error += dr*dr + dg*dg + db*db;
            }
        }
    }

    return error;
}

void PrintTriangle(Triangle& t, const char* label = "")
{
    std::cout << label
              << " p0=(" << t.p0.x << ", " << t.p0.y << ")"
              << " p1=(" << t.p1.x << ", " << t.p1.y << ")"
              << " p2=(" << t.p2.x << ", " << t.p2.y << ")"
              << " color=("
              << int(t.color.r) << ", "
              << int(t.color.g) << ", "
              << int(t.color.b) << ", "
              << int(t.color.a) << ")"
              << std::endl;
}

int main() {
  std::cout << "CWD: " << std::filesystem::current_path() << "\n";
  srand(time(0));
  int screenWidth = 1980;
  int screenHeight = 1080;

  raylib::Window window(screenWidth, screenHeight, "raylib-cpp - basic window");

  window.SetFullscreen(true);

  Image orgImg = LoadImage("input.png");    // keep in memory
  
  ImageResize(&orgImg, orgImg.width / 4, orgImg.height/4);

  Texture2D originalTex = LoadTextureFromImage(orgImg);
  RenderTexture2D original = LoadRenderTexture(screenWidth, screenHeight);

  BeginTextureMode(original);
  DrawTexture(originalTex, 0, 0, WHITE);
  EndTextureMode();

  RenderTexture2D accum = LoadRenderTexture(screenWidth, screenHeight);

  RenderTexture2D currentTex;
  Image currentImg = LoadImageFromTexture(currentTex.texture);
  
  Shader avgShader = LoadShader(0, "src/triangle_avg.fs");

  SetTargetFPS(60);
  
  // clear it to black initially
  BeginDrawing();
  ClearBackground(BLACK);
  EndDrawing();
  while (!window.ShouldClose()) {
    std::array<Triangle, NUM_TRIANGLES> triangles;
    for (int i = 0; i < NUM_TRIANGLES; ++i) {
      triangles[i] = GenerateRandomTriangle(orgImg);
    }
    for (int i = 0; i < NUM_TRIANGLES; ++i) {
      triangles[i].color = ComputeTriangleAvgColor_CPU(triangles[i], orgImg);
    }
    
    // pick the best triangle
    Triangle best = triangles[0];
    float bestError = TriangleError(best, original, currentImg);

    for (int i = 1; i < NUM_TRIANGLES; ++i) {
      float err = TriangleError(triangles[i], original, currentImg);
      if (err > bestError) {
          bestError = err;
          best = triangles[i];
      }
    }
    std::cout << "done\n";
  

    BeginTextureMode(currentTex);
    DrawTriangle(best.p0, best.p1, best.p2, best.color);
    EndTextureMode();
  
    Image currentImg = LoadImageFromTexture(currentTex.texture);

    BeginDrawing();
    DrawTexture(currentTex.texture, 0, 0, WHITE);
    EndDrawing();
  }



  return 0;
}
