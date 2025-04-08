#include <iostream>

#define SDL_MAIN_HANDLED
#include "mgui.cpp"

int main() {
    State state = State("test", 640, 480, 120, WINDOW_RESIZEABLE | INPUT_MULTI_THREADED | ELEMENTS_ENABLE);
    state.r.init_frame_buffer();
    uint32_t* pixels = (uint32_t*)malloc(state.r.w * state.r.h * sizeof(uint32_t));
    char r = 69;
    char g = 69;
    char b = 69;
    memset(pixels, 0xFF0000FF, state.r.w * state.r.h * sizeof(uint32_t));
    while (!state.quit) {
        state.start_frame();
        state.r.set_frame_buffer(pixels);
        state.end_frame();
    }
    free(pixels);
    state.r.destroy_frame_buffer();
}