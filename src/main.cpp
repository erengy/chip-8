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

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "chip8.h"
#include "sdl.h"

constexpr uint8_t kDisplayMultiplier = 10;

static chip8::Emulator emulator;

class Engine : public sdl::Engine {
public:
  void OnKeyEvent(SDL_KeyboardEvent key_event);
  void OnLoop();
  void OnRender();
};

void Engine::OnKeyEvent(SDL_KeyboardEvent key_event) {
  if (key_event.repeat != 0)
    return;

  switch (key_event.keysym.sym) {
    case SDLK_ESCAPE:
      running_ = false;
      return;
    case SDLK_F5:
      // TODO: Restart
      return;
  }

  static const std::map<SDL_Keycode, uint8_t> key_map = {
    {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},  // 1|2|3|C
    {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},  // 4|5|6|D
    {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},  // 7|8|9|E
    {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},  // A|0|B|F
  };
  auto it = key_map.find(key_event.keysym.sym);
  if (it != key_map.end())
    emulator.SetKey(it->second, key_event.state == SDL_PRESSED);
}

void Engine::OnLoop() {
  emulator.Cycle();

  if (emulator.speaker)
    std::cout << '\a';
}

void Engine::OnRender() {
  SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_, nullptr);

  SDL_Rect rect = {0, 0, kDisplayMultiplier, kDisplayMultiplier};

  for (uint8_t y = 0; y < chip8::kDisplayHeight; ++y) {
    for (uint8_t x = 0; x < chip8::kDisplayWidth; ++x) {
      if (emulator.GetPixel(x, y)) {
        rect.x = x * kDisplayMultiplier;
        rect.y = y * kDisplayMultiplier;
        SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer_, &rect);
      }
    }
  }
}

static bool ReadFile(const std::string& path, std::vector<uint8_t>& data) {
  std::ifstream is;
  is.open(path.c_str(), std::ios::binary);

  is.seekg(0, std::ios::end);
  size_t len = static_cast<size_t>(is.tellg());

  if (len != -1) {
    data.resize(len);
    is.seekg(0, std::ios::beg);
    is.read((char*)data.data(), data.size());
  }

  is.close();
  return len != -1;
}

int main(int argc, char const *argv[]) {
  if (argc < 2)
    return 1;

  const std::string path = argv[1];
  const std::string filename = path.substr(path.find_last_of("/\\") + 1);
  if (path.empty() || filename.empty())
    return 1;

  std::vector<uint8_t> data;
  if (!ReadFile(path, data))
    return 1;

  emulator.Reset();
  emulator.Load(data);

  Engine engine;
  if (!engine.Initialize() ||
      !engine.CreateWindow("CHIP-8 Emulator [" + filename + "]",
                           chip8::kDisplayWidth * kDisplayMultiplier,
                           chip8::kDisplayHeight * kDisplayMultiplier) ||
      !engine.CreateRenderer()) {
    return 1;
  }

  engine.Loop();

  return 0;
}
