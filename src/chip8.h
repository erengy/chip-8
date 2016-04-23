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

#include <array>
#include <cstdint>
#include <vector>

namespace chip8 {

constexpr uint8_t kDefaultSpriteHeight = 5;
constexpr uint8_t kDisplayHeight = 32;
constexpr uint8_t kDisplayWidth = 64;
constexpr uint16_t kProgramOffset = 0x200;

typedef std::array<bool, kDisplayWidth * kDisplayHeight> display_t;
typedef std::array<bool, 16> input_t;
typedef std::array<uint8_t, 4096> memory_t;

struct Processor {
  std::array<uint8_t, 16> v;       // 8-bit registers
  uint16_t i = 0;                  // 16-bit register
  uint16_t pc = kProgramOffset;    // program counter
  uint8_t sp = 0;                  // stack pointer
  std::array<uint16_t, 16> stack;
  uint8_t dt = 0;                  // delay timer
  uint8_t st = 0;                  // sound timer
};

class Emulator {
public:
  void Cycle();
  bool Load(const std::vector<uint8_t>& data);
  void Reset();

  bool GetPixel(uint8_t x, uint8_t y) const;
  void SetKey(uint8_t key, bool pressed);

private:
  display_t display_;
  input_t input_;
  memory_t memory_;
  Processor processor_;
};

}  // namespace chip8
