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

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <tuple>

#include "chip8.h"

namespace chip8 {

void Emulator::Cycle() {
  instruction_ = memory[processor.pc] << 8 | memory[processor.pc + 1];

  increment_pc();

  typedef std::tuple<uint16_t, uint16_t, std::function<void()>> instruction_t;
  static const std::vector<instruction_t> instructions = {
    {0x00E0, 0xFFFF, std::bind(&Emulator::op_00E0, this)},
    {0x00EE, 0xFFFF, std::bind(&Emulator::op_00EE, this)},
    {0x1000, 0xF000, std::bind(&Emulator::op_1nnn, this)},
    {0x2000, 0xF000, std::bind(&Emulator::op_2nnn, this)},
    {0x3000, 0xF000, std::bind(&Emulator::op_3xkk, this)},
    {0x4000, 0xF000, std::bind(&Emulator::op_4xkk, this)},
    {0x5000, 0xF00F, std::bind(&Emulator::op_5xy0, this)},
    {0x6000, 0xF000, std::bind(&Emulator::op_6xkk, this)},
    {0x7000, 0xF000, std::bind(&Emulator::op_7xkk, this)},
    {0x8000, 0xF00F, std::bind(&Emulator::op_8xy0, this)},
    {0x8001, 0xF00F, std::bind(&Emulator::op_8xy1, this)},
    {0x8002, 0xF00F, std::bind(&Emulator::op_8xy2, this)},
    {0x8003, 0xF00F, std::bind(&Emulator::op_8xy3, this)},
    {0x8004, 0xF00F, std::bind(&Emulator::op_8xy4, this)},
    {0x8005, 0xF00F, std::bind(&Emulator::op_8xy5, this)},
    {0x8006, 0xF00F, std::bind(&Emulator::op_8xy6, this)},
    {0x8007, 0xF00F, std::bind(&Emulator::op_8xy7, this)},
    {0x800E, 0xF00F, std::bind(&Emulator::op_8xyE, this)},
    {0x9000, 0xF00F, std::bind(&Emulator::op_9xy0, this)},
    {0xA000, 0xF000, std::bind(&Emulator::op_Annn, this)},
    {0xB000, 0xF000, std::bind(&Emulator::op_Bnnn, this)},
    {0xC000, 0xF000, std::bind(&Emulator::op_Cxkk, this)},
    {0xD000, 0xF000, std::bind(&Emulator::op_Dxyn, this)},
    {0xE09E, 0xF0FF, std::bind(&Emulator::op_Ex9E, this)},
    {0xE0A1, 0xF0FF, std::bind(&Emulator::op_ExA1, this)},
    {0xF007, 0xF0FF, std::bind(&Emulator::op_Fx07, this)},
    {0xF00A, 0xF0FF, std::bind(&Emulator::op_Fx0A, this)},
    {0xF015, 0xF0FF, std::bind(&Emulator::op_Fx15, this)},
    {0xF018, 0xF0FF, std::bind(&Emulator::op_Fx18, this)},
    {0xF01E, 0xF0FF, std::bind(&Emulator::op_Fx1E, this)},
    {0xF029, 0xF0FF, std::bind(&Emulator::op_Fx29, this)},
    {0xF033, 0xF0FF, std::bind(&Emulator::op_Fx33, this)},
    {0xF055, 0xF0FF, std::bind(&Emulator::op_Fx55, this)},
    {0xF065, 0xF0FF, std::bind(&Emulator::op_Fx65, this)},
  };

  bool unknown_instruction = true;
  for (const auto& instruction : instructions) {
    if ((instruction_ & std::get<1>(instruction)) == std::get<0>(instruction)) {
      std::get<2>(instruction)();
      unknown_instruction = false;
      break;
    }
  }
  if (unknown_instruction)
    op_unknown();

  if (processor.dt > 0) {
    --processor.dt;
  }
  if (processor.st > 0) {
    speaker = true;
    --processor.st;
  } else {
    speaker = false;
  }
}

bool Emulator::Load(const std::vector<uint8_t>& program) {
  if (program.size() > memory.size() - kProgramOffset)
    return false;

  std::copy(program.begin(), program.end(), std::back_inserter(program_));
  std::copy(program.begin(), program.end(), memory.begin() + kProgramOffset);

  return true;
}

void Emulator::Reset() {
  display.fill(false);
  input.fill(false);
  memory.fill(0);

  processor.v.fill(0);
  processor.i = 0;
  processor.pc = kProgramOffset;
  processor.sp = 0;
  processor.stack.fill(0);
  processor.dt = 0;
  processor.st = 0;

  static const std::array<uint8_t, 16 * 5> sprites = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
  };
  std::copy(sprites.begin(), sprites.end(), memory.begin());

  instruction_ = 0x0000;
  program_.clear();
}

void Emulator::Restart() {
  const auto program = program_;
  Reset();
  Load(program);
}

bool Emulator::GetPixel(uint8_t x, uint8_t y) const {
  const size_t pixel = x + (kDisplayWidth * y);
  if (pixel < display.size())
    return display[pixel];
  return false;
}

void Emulator::SetKey(uint8_t key, bool pressed) {
  if (key < input.size())
    input[key] = pressed;
}

////////////////////////////////////////////////////////////////////////////////

void Emulator::op_00E0() {  // CLS
  display.fill(false);
}

void Emulator::op_00EE() {  // RET
  if (processor.sp > 0) {
    processor.pc = processor.stack[--processor.sp];
  } else {
    std::cout << "Stack underflow\n";
  }
}

void Emulator::op_1nnn() {  // JP addr
  processor.pc = get_addr();
}

void Emulator::op_2nnn() {  // CALL addr
  if (processor.sp < processor.stack.size()) {
    processor.stack[processor.sp++] = processor.pc;
    processor.pc = get_addr();
  } else {
    std::cout << "Stack overflow\n";
  }
}

void Emulator::op_3xkk() {  // SE Vx, byte
  if (vx() == get_byte())
    increment_pc();
}

void Emulator::op_4xkk() {  // SNE Vx, byte
  if (vx() != get_byte())
    increment_pc();
}

void Emulator::op_5xy0() {  // SE Vx, Vy
  if (vx() == vy())
    increment_pc();
}

void Emulator::op_6xkk() {  // LD Vx, byte
  vx() = get_byte();
}

void Emulator::op_7xkk() {  // ADD Vx, byte
  vx() += get_byte();
}

void Emulator::op_8xy0() {  // LD Vx, Vy
  vx() = vy();
}

void Emulator::op_8xy1() {  // OR Vx, Vy
  vx() |= vy();
}

void Emulator::op_8xy2() {  // AND Vx, Vy
  vx() &= vy();
}

void Emulator::op_8xy3() {  // XOR Vx, Vy
  vx() ^= vy();
}

void Emulator::op_8xy4() {  // ADD Vx, Vy
  vx() += vy();
  vf() = vx() > 0x00FF ? 1 : 0;
  vx() &= 0x00FF;
}

void Emulator::op_8xy5() {  // SUB Vx, Vy
  vf() = vx() > vy() ? 1 : 0;
  vx() -= vy();
}

void Emulator::op_8xy6() {  // SHR Vx {, Vy}
  vf() = vx() & 1 ? 1 : 0;
  vx() >>= 1;
}

void Emulator::op_8xy7() {  // SUBN Vx, Vy
  vf() = vy() > vx() ? 1 : 0;
  vx() = vy() - vx();
}

void Emulator::op_8xyE() {  // SHL Vx {, Vy}
  vf() = static_cast<uint16_t>(vx()) >> 15 ? 1 : 0;
  vx() <<= 1;
}

void Emulator::op_9xy0() {  // SNE Vx, Vy
  if (vx() != vy())
    increment_pc();
}

void Emulator::op_Annn() {  // LD I, addr
  processor.i = get_addr();
}

void Emulator::op_Bnnn() {  // JP V0, addr
  processor.pc = get_addr() + processor.v[0];
}

void Emulator::op_Cxkk() {  // RND Vx, byte
  std::random_device random_device;
  std::mt19937 mt19937(random_device());
  std::uniform_int_distribution<uint16_t> distribution(0, 255);
  vx() = static_cast<uint8_t>(distribution(mt19937)) & get_byte();
}

void Emulator::op_Dxyn() {  // DRW Vx, Vy, nibble
  bool collision = false;

  for (uint8_t y = 0; y < get_nibble(); ++y) {
    const auto sprite = memory[processor.i + y];
    for (uint8_t x = 0; x < 8; ++x) {
      const uint8_t pos_x = (vx() + x) % kDisplayWidth;
      const uint8_t pos_y = (vy() + y) % kDisplayHeight;
      auto& pixel = display[pos_x + (kDisplayWidth * pos_y)];
      bool new_pixel = pixel ^ ((sprite >> (7 - x)) & 1);
      collision |= pixel && !new_pixel;
      pixel = new_pixel;
    }
  }

  vf() = collision ? 1 : 0;
}

void Emulator::op_Ex9E() {  // SKP Vx
  if (input[vx()])
    increment_pc();
}

void Emulator::op_ExA1() {  // SKNP Vx
  if (!input[vx()])
    increment_pc();
}

void Emulator::op_Fx07() {  // LD Vx, DT
  vx() = processor.dt;
}

void Emulator::op_Fx0A() {  // LD Vx, K
  vx() = 0x0;  // TODO: Store pressed key in Vx
  op_unknown();
}

void Emulator::op_Fx15() {  // LD DT, Vx
  processor.dt = vx();
}

void Emulator::op_Fx18() {  // LD ST, Vx
  processor.st = vx();
}

void Emulator::op_Fx1E() {  // ADD I, Vx
  processor.i += vx();
}

void Emulator::op_Fx29() {  // LD F, Vx
  processor.i = vx() * kDefaultSpriteHeight;
}

void Emulator::op_Fx33() {  // LD B, Vx
  memory[processor.i + 0] = vx() / 100;
  memory[processor.i + 1] = (vx() / 10) % 10;
  memory[processor.i + 2] = vx() % 10;
}

void Emulator::op_Fx55() {  // LD [I], Vx
  for (uint8_t j = 0; j <= ((instruction_ & 0x0F00) >> 8); ++j) {
    memory[processor.i + j] = processor.v[j];
  }
}

void Emulator::op_Fx65() {  // LD Vx, [I]
  for (uint8_t j = 0; j <= ((instruction_ & 0x0F00) >> 8); ++j) {
    processor.v[j] = memory[processor.i + j];
  }
}

void Emulator::op_unknown() {
  std::cout << "Unknown instruction: 0x" << std::hex << instruction_ << "\n";
}

inline uint16_t Emulator::get_addr() const {
  return static_cast<uint16_t>(instruction_ & 0x0FFF);
};

inline uint8_t Emulator::get_byte() const {
  return static_cast<uint8_t>(instruction_ & 0x00FF);
};

inline uint8_t Emulator::get_nibble() const {
  return static_cast<uint8_t>(instruction_ & 0x000F);
};

inline void Emulator::increment_pc() {
  processor.pc += sizeof(instruction_);  // 2
};

inline uint8_t& Emulator::vf() {
  return processor.v[0xF];
};

inline uint8_t& Emulator::vx()  {
  return processor.v[(instruction_ & 0x0F00) >> 8];
};

inline uint8_t& Emulator::vy() {
  return processor.v[(instruction_ & 0x00F0) >> 4];
};

}  // namespace chip8
