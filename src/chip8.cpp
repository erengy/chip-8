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
#include <iostream>
#include <random>

#include "chip8.h"

namespace chip8 {

void Emulator::Cycle() {
  const uint16_t instruction =
      memory_[processor_.pc] << 8 | memory_[processor_.pc + 1];

  auto get_random = []() {
    std::random_device random_device;
    std::mt19937 mt19937(random_device());
    std::uniform_int_distribution<uint8_t> distribution(0, 255);
    return distribution(mt19937);
  };
  auto get_nnn = [&instruction]() {
    return static_cast<uint16_t>(instruction & 0x0FFF);
  };
  auto get_kk = [&instruction]() {
    return static_cast<uint8_t>(instruction & 0x00FF);
  };
  auto get_n = [&instruction]() {
    return static_cast<uint8_t>(instruction & 0x000F);
  };
  auto increment_pc = [this, &instruction]() {
    processor_.pc += sizeof(instruction);  // 2
  };
  auto vf = [this]() -> uint8_t& {
    return processor_.v[0xF];
  };
  auto vx = [this, &instruction]() -> uint8_t& {
    return processor_.v[(instruction & 0x0F00) >> 8];
  };
  auto vy = [this, &instruction]() -> uint8_t& {
    return processor_.v[(instruction & 0x00F0) >> 4];
  };

  increment_pc();

  switch (instruction & 0xF000) {
    case 0x0000:
      switch (get_kk()) {
        case 0xE0:  // CLS
          display_.fill(false);
          break;
        case 0xEE:  // RET
          if (processor_.sp > 0) {
            processor_.pc = processor_.stack[--processor_.sp];
          } else {
            // TODO: Handle stack underflow
          }
          break;
      }
      break;

    case 0x1000:  // JP addr
      processor_.pc = get_nnn();
      break;

    case 0x2000:  // CALL addr
      if (processor_.sp < processor_.stack.size()) {
        processor_.stack[processor_.sp++] = processor_.pc;
        processor_.pc = get_nnn();
      } else {
        // TODO: Handle stack overflow
      }
      break;

    case 0x3000:  // SE Vx, byte
      if (vx() == get_kk())
        increment_pc();
      break;

    case 0x4000:  // SNE Vx, byte
      if (vx() != get_kk())
        increment_pc();
      break;

    case 0x5000:  // SE Vx, Vy
      if (vx() == vy())
        increment_pc();
      break;

    case 0x6000:  // LD Vx, byte
      vx() = get_kk();
      break;

    case 0x7000:  // ADD Vx, byte
      vx() += get_kk();
      break;

    case 0x8000:
      switch (get_n()) {
        case 0x0:  // LD Vx, Vy
          vx() = vy();
          break;
        case 0x1:  // OR Vx, Vy
          vx() |= vy();
          break;
        case 0x2:  // AND Vx, Vy
          vx() &= vy();
          break;
        case 0x3:  // XOR Vx, Vy
          vx() ^= vy();
          break;
        case 0x4:  // ADD Vx, Vy
          vx() += vy();
          vf() = vx() > 0x00FF ? 1 : 0;
          vx() &= 0x00FF;
          break;
        case 0x5:  // SUB Vx, Vy
          vf() = vx() > vy() ? 1 : 0;
          vx() -= vy();
          break;
        case 0x6:  // SHR Vx {, Vy}
          vf() = vx() & 1 ? 1 : 0;
          vx() >>= 1;
          break;
        case 0x7:  // SUBN Vx, Vy
          vf() = vy() > vx() ? 1 : 0;
          vx() = vy() - vx();
          break;
        case 0xE:  // SHL Vx {, Vy}
          vf() = vx() >> 15 ? 1 : 0;
          vx() <<= 1;
          break;
      }
      break;

    case 0x9000:  // SNE Vx, Vy
      if (vx() != vy())
        increment_pc();
      break;

    case 0xA000:  // LD I, addr
      processor_.i = get_nnn();
      break;

    case 0xB000:  // JP V0, addr
      processor_.pc = get_nnn() + processor_.v[0];
      break;

    case 0xC000:  // RND Vx, byte
      vx() = get_random() & get_kk();
      break;

    case 0xD000: {  // DRW Vx, Vy, nibble
      bool collision = false;
      for (uint8_t y = 0; y < get_n(); ++y) {
        const auto sprite = memory_[processor_.i + y];
        for (uint8_t x = 0; x < 8; ++x) {
          const uint8_t pos_x = (vx() + x) % kDisplayWidth;
          const uint8_t pos_y = (vy() + y) % kDisplayHeight;
          auto& pixel = display_[pos_x + (kDisplayWidth * pos_y)];
          bool new_pixel = pixel ^ ((sprite >> (7 - x)) & 1);
          if (pixel && !new_pixel)
            collision = true;
          pixel = new_pixel;
        }
      }
      vf() = collision ? 1 : 0;
      break;
    }

    case 0xE000:
      switch (get_kk()) {
        case 0x9E:  // SKP Vx
          if (input_[vx()])
            increment_pc();
          break;
        case 0xA1:  // SKNP Vx
          if (!input_[vx()])
            increment_pc();
          break;
      }
      break;

    case 0xF000:
      switch (get_kk()) {
        case 0x07:  // LD Vx, DT
          vx() = processor_.dt;
          break;
        case 0x0A:  // LD Vx, K
          std::cout << "LD Vx, K" << std::endl;
          vx() = 0x0;  // TODO: Store pressed key in Vx
          break;
        case 0x15:  // LD DT, Vx
          processor_.dt = vx();
          break;
        case 0x18:  // LD ST, Vx
          processor_.st = vx();
          break;
        case 0x1E:  // ADD I, Vx
          processor_.i += vx();
          break;
        case 0x29:  // LD F, Vx
          processor_.i = vx() * kDefaultSpriteHeight;
          break;
        case 0x33:  // LD B, Vx
          memory_[processor_.i + 0] = vx() / 100;
          memory_[processor_.i + 1] = (vx() / 10) % 10;
          memory_[processor_.i + 2] = vx() % 10;
          break;
        case 0x55:  // LD [I], Vx
          for (uint8_t i = 0; i <= ((instruction & 0x0F00) >> 8); ++i) {
            memory_[processor_.i + i] = processor_.v[i];
          }
          break;
        case 0x65:  // LD Vx, [I]
          for (uint8_t i = 0; i <= ((instruction & 0x0F00) >> 8); ++i) {
            processor_.v[i] = memory_[processor_.i + i];
          }
          break;
      }
      break;
  }

  if (processor_.dt > 0) {
    --processor_.dt;
  }
  if (processor_.st > 0) {
    if (processor_.st == 1)
      std::cout << '\a';  // TODO: Beep with SDL
    --processor_.st;
  }
}

bool Emulator::Load(const std::vector<uint8_t>& data) {
  if (data.size() > memory_.size() - kProgramOffset)
    return false;

  std::copy(data.begin(), data.end(), memory_.begin() + kProgramOffset);
  return true;
}

void Emulator::Reset() {
  display_.fill(false);
  input_.fill(false);
  memory_.fill(0);

  processor_.v.fill(0);
  processor_.i = 0;
  processor_.pc = kProgramOffset;
  processor_.sp = 0;
  processor_.stack.fill(0);
  processor_.dt = 0;
  processor_.st = 0;

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
  std::copy(sprites.begin(), sprites.end(), memory_.begin());
}

bool Emulator::GetPixel(uint8_t x, uint8_t y) const {
  const size_t pixel = x + (kDisplayWidth * y);
  if (pixel < display_.size())
    return display_[pixel];
  return false;
}

void Emulator::SetKey(uint8_t key, bool pressed) {
  if (key < input_.size())
    input_[key] = pressed;
}

}  // namespace chip8
