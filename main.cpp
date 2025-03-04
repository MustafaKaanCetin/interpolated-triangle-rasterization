#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <iostream>

int width = 800;
int height = 600;

void putpixel(int x, int y, const std::vector<int>& color, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], color[3]);
    SDL_RenderDrawPoint(renderer, x, y);
}

std::vector<int> getpixelcolor(int x, int y, SDL_Renderer* renderer) {
    std::vector<int> color(4,0);
    uint32_t pixel;

    // Allocate a 1-pixel buffer (4 bytes per pixel)
    SDL_Rect rect = {x, y, 1, 1};
    if (SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_RGBA32, &pixel, sizeof(uint32_t)) != 0) {
        std::cerr << "SDL_RenderReadPixels failed: " << SDL_GetError() << std::endl;
        return color;  // Return black if failed
    }

    // Extract color components
    Uint8 r, g, b, a;
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
    SDL_FreeFormat(format);

    // Store in vector
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    return color;
}

std::vector<int> scanline(int y, int xmin, int xmax, SDL_Renderer* renderer) {
    std::vector<int> drawnpixels = std::vector<int>();
    for (int i = xmin; i <= xmax; i++) {
        std::vector<int> pixcol = getpixelcolor(i, y, renderer);
        if (pixcol[0] || pixcol[1] || pixcol[2]) {
            drawnpixels.push_back(i);
        }
    }
    return drawnpixels;
}


//Bresenham's
void drawline(int x0, int y0, int x1, int y1, std::vector<int> color0, std::vector<int> color1, SDL_Renderer* renderer) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int length = std::max(dx, dy);

    if (length == 0) {
        putpixel(x0, y0, color0, renderer);
        return;
    }

    for (int i = 0; i < length; i++) {
        float n = (float)(i) / (float)(length - 1);
        std::vector<int> color = std::vector<int>(4);
        for (int j = 0; j < 3; j++) {
            color[j] = color0[j]*(1-n) + color1[j]*n;
        }
        putpixel(x0, y0, color, renderer);

        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawtriangle(int x0, int y0, int x1, int y1, int x2, int y2, SDL_Renderer* renderer) {
    putpixel(x0, y0, {255, 0, 0, 255}, renderer);
    putpixel(x1, y1, {0, 255, 0, 255}, renderer);
    putpixel(x2, y2, {0, 0, 255, 255}, renderer);
    drawline(x0, y0, x1, y1, getpixelcolor(x0, y0, renderer), getpixelcolor(x1,y1,renderer), renderer);
    drawline(x1, y1, x2, y2, getpixelcolor(x1, y1, renderer), getpixelcolor(x2, y2, renderer), renderer);
    drawline(x2, y2, x0, y0, getpixelcolor(x2, y2, renderer), getpixelcolor(x0, y0, renderer), renderer);
    std::vector<int> color = getpixelcolor(x0, y0, renderer);
    int minx = std::min(x0, std::min(x1, x2));
    int miny = std::min(y0, std::min(y1, y2));
    int maxx = std::max(x0, std::max(x1, x2));
    int maxy = std::max(y0, std::max(y1, y2));
    for (int y = miny; y <= maxy; y++) {
        std::vector<int> index = scanline(y, minx, maxx, renderer);
        if (index.empty()) {
            continue;
        }
        int start = index[0];
        int end = index[index.size() - 1];
        std::vector<int> color = getpixelcolor(start, y, renderer);
        drawline(start, y, end, y, getpixelcolor(start, y, renderer), getpixelcolor(end, y, renderer), renderer);
    }
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window  *window;
    SDL_Renderer* renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &window, &renderer);
    SDL_RenderSetLogicalSize(renderer, width, height);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawtriangle(100, 100, 400, 500, 700, 200, renderer);
    SDL_RenderPresent(renderer);

    while (true) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            break;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}