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

#pragma once

#include <string>

#define SDL_MAIN_HANDLED 1
#include <SDL.h>

namespace sdl {

class Engine {
public:
  ~Engine();

  const SDL_Window* window() const;
  const SDL_Renderer* renderer() const;

  bool Initialize();
  bool CreateWindow(const std::string& title, int width, int height);
  bool CreateRenderer();

  bool OpenAudioDevice(const SDL_AudioSpec& audio_spec);
  void PauseAudioDevice(int pause_on) const;
  bool QueueAudio(const void* data, Uint32 len) const;

  void Loop();

  virtual void OnKeyEvent(SDL_KeyboardEvent key_event) {}
  virtual void OnLoop() {}
  virtual void OnRender() {}

protected:
  SDL_AudioDeviceID audio_device_ = 0;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  bool running_ = false;
};

class Timer {
public:
  Timer(Uint32 target);

  bool Check();

private:
  Uint64 counter_ = 0;
  Uint64 frequency_ = 0;
  Uint32 target_ = 0;
};

}  // namespace sdl
