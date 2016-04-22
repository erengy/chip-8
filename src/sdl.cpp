/*
MIT License

Copyright (c) 2016 Eren Okka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "sdl.h"

namespace sdl {

Engine::~Engine() {
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }

  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
}

const SDL_Window* Engine::window() const {
  return window_;
}

const SDL_Renderer* Engine::renderer() const {
  return renderer_;
}

bool Engine::Initialize() {
  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
  return SDL_Init(flags) == 0;
}

bool Engine::CreateWindow(const std::string& title, int width, int height) {
  window_ = SDL_CreateWindow(title.c_str(),
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             width, height,
                             SDL_WINDOW_SHOWN);
  return window_ != nullptr;
}

bool Engine::CreateRenderer() {
  // TODO: Disable v-sync after implementing frame-rate limit
  Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  renderer_ = SDL_CreateRenderer(window_, -1, flags);
  return renderer_ != nullptr;
}

void Engine::Loop() {
  SDL_Event e;
  running_ = true;

  while (running_) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          running_ = false;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          OnKeyEvent(e.key);
          break;
      }
    }

    OnLoop();
    Render();
  }
}

void Engine::Render() {
  SDL_RenderClear(renderer_);
  OnRender();
  SDL_RenderPresent(renderer_);
}

}  // namespace sdl
