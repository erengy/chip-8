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
typedef bool speaker_t;

struct Processor {
  std::array<uint8_t, 16> v;       // 8-bit registers
  uint16_t i = 0;                  // 16-bit register
  uint16_t pc = kProgramOffset;    // program counter
  uint8_t sp = 0;                  // stack pointer
  std::array<uint16_t, 16> stack;
  uint8_t dt = 0;                  // delay timer
  uint8_t st = 0;                  // sound timer
};

struct Machine {
  display_t display;
  input_t input;
  memory_t memory;
  Processor processor;
  speaker_t speaker;
};

class Emulator : public Machine {
public:
  void Cycle();
  bool Load(const std::vector<uint8_t>& data);
  void Reset();

  bool GetPixel(uint8_t x, uint8_t y) const;
  void SetKey(uint8_t key, bool pressed);

private:
  void op_00E0();
  void op_00EE();
  void op_1nnn();
  void op_2nnn();
  void op_3xkk();
  void op_4xkk();
  void op_5xy0();
  void op_6xkk();
  void op_7xkk();
  void op_8xy0();
  void op_8xy1();
  void op_8xy2();
  void op_8xy3();
  void op_8xy4();
  void op_8xy5();
  void op_8xy6();
  void op_8xy7();
  void op_8xyE();
  void op_9xy0();
  void op_Annn();
  void op_Bnnn();
  void op_Cxkk();
  void op_Dxyn();
  void op_Ex9E();
  void op_ExA1();
  void op_Fx07();
  void op_Fx0A();
  void op_Fx15();
  void op_Fx18();
  void op_Fx1E();
  void op_Fx29();
  void op_Fx33();
  void op_Fx55();
  void op_Fx65();
  void op_unknown();

  inline uint16_t get_addr() const;
  inline uint8_t get_byte() const;
  inline uint8_t get_nibble() const;
  inline void increment_pc();
  inline uint8_t& vf();
  inline uint8_t& vx();
  inline uint8_t& vy();

  uint16_t instruction_;
};

}  // namespace chip8
