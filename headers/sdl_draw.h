// sdl_draw.h
#ifndef SDL_DRAW_H
#define SDL_DRAW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
} SDL_DrawContext;

// Initialize SDL and create a window
SDL_DrawContext *sdl_init(const char *title, int width, int height, const char *font_path, int font_size);

// Draw a single pixel
void sdl_draw_pixel(SDL_DrawContext *ctx, int x, int y, SDL_Color color);

// Draw text at a specific position
void sdl_draw_text(SDL_DrawContext *ctx, const char *text, int x, int y, SDL_Color color);

// Clear the screen
void sdl_clear(SDL_DrawContext *ctx, SDL_Color color);

// Render everything to the screen
void sdl_present(SDL_DrawContext *ctx);

// Clean up and free resources
void sdl_destroy(SDL_DrawContext *ctx);

#endif // SDL_DRAW_H
