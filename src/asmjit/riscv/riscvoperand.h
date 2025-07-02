// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED
#define ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED

#include "../core/operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \addtogroup asmjit_riscv
//! \{

//! Register traits.
//!
//! RISC-V register identifiers are composed of Group and Type.
struct RegTraits : public RegTraitsTemplate<RegType> {
  enum : uint32_t { kSignature = 0x00010000 };

  static inline constexpr RegType typeToSignature(RegType type) noexcept {
    return RegType(uint32_t(type) | kSignature);
  }
};

//! General purpose register.
class Gp : public Reg {
public:
  ASMJIT_DEFINE_REG_TRAITS(Gp, RegTraits, RegGroup::kGp)

  //! Tests whether the register is a `W` register.
  ASMJIT_INLINE_NODEBUG bool isW() const noexcept { return hasSignature(RegTraits::typeToSignature(RegType::kGp32)); }
  //! Tests whether the register is an `X` register.
  ASMJIT_INLINE_NODEBUG bool isX() const noexcept { return hasSignature(RegTraits::typeToSignature(RegType::kGp64)); }
};

namespace regs {

//! Creates a `w` register operand.
static ASMJIT_INLINE_NODEBUG constexpr Gp w(uint32_t id) noexcept { return Gp(Gp::kSignature | (uint32_t(RegType::kGp32) << 8) | id); }
//! Creates a `x` register operand.
static ASMJIT_INLINE_NODEBUG constexpr Gp x(uint32_t id) noexcept { return Gp(Gp::kSignature | (uint32_t(RegType::kGp64) << 8) | id); }

// 32-bit Registers.
static constexpr Gp w0 = w(0);
static constexpr Gp w1 = w(1);
static constexpr Gp w2 = w(2);
static constexpr Gp w3 = w(3);
static constexpr Gp w4 = w(4);
static constexpr Gp w5 = w(5);
static constexpr Gp w6 = w(6);
static constexpr Gp w7 = w(7);
static constexpr Gp w8 = w(8);
static constexpr Gp w9 = w(9);
static constexpr Gp w10 = w(10);
static constexpr Gp w11 = w(11);
static constexpr Gp w12 = w(12);
static constexpr Gp w13 = w(13);
static constexpr Gp w14 = w(14);
static constexpr Gp w15 = w(15);
static constexpr Gp w16 = w(16);
static constexpr Gp w17 = w(17);
static constexpr Gp w18 = w(18);
static constexpr Gp w19 = w(19);
static constexpr Gp w20 = w(20);
static constexpr Gp w21 = w(21);
static constexpr Gp w22 = w(22);
static constexpr Gp w23 = w(23);
static constexpr Gp w24 = w(24);
static constexpr Gp w25 = w(25);
static constexpr Gp w26 = w(26);
static constexpr Gp w27 = w(27);
static constexpr Gp w28 = w(28);
static constexpr Gp w29 = w(29);
static constexpr Gp w30 = w(30);
static constexpr Gp w31 = w(31);

// 64-bit Registers.
static constexpr Gp x0 = x(0);   // zero.
static constexpr Gp x1 = x(1);   // ra.
static constexpr Gp x2 = x(2);   // sp.
static constexpr Gp x3 = x(3);   // gp.
static constexpr Gp x4 = x(4);   // tp.
static constexpr Gp x5 = x(5);   // t0.
static constexpr Gp x6 = x(6);   // t1.
static constexpr Gp x7 = x(7);   // t2.
static constexpr Gp x8 = x(8);   // s0/fp.
static constexpr Gp x9 = x(9);   // s1.
static constexpr Gp x10 = x(10); // a0.
static constexpr Gp x11 = x(11); // a1.
static constexpr Gp x12 = x(12); // a2.
static constexpr Gp x13 = x(13); // a3.
static constexpr Gp x14 = x(14); // a4.
static constexpr Gp x15 = x(15); // a5.
static constexpr Gp x16 = x(16); // a6.
static constexpr Gp x17 = x(17); // a7.
static constexpr Gp x18 = x(18); // s2.
static constexpr Gp x19 = x(19); // s3.
static constexpr Gp x20 = x(20); // s4.
static constexpr Gp x21 = x(21); // s5.
static constexpr Gp x22 = x(22); // s6.
static constexpr Gp x23 = x(23); // s7.
static constexpr Gp x24 = x(24); // s8.
static constexpr Gp x25 = x(25); // s9.
static constexpr Gp x26 = x(26); // s10.
static constexpr Gp x27 = x(27); // s11.
static constexpr Gp x28 = x(28); // t3.
static constexpr Gp x29 = x(29); // t4.
static constexpr Gp x30 = x(30); // t5.
static constexpr Gp x31 = x(31); // t6.

// Named Registers.
static constexpr Gp zero = x0;
static constexpr Gp ra = x1;
static constexpr Gp sp = x2;
static constexpr Gp gp = x3;
static constexpr Gp tp = x4;
static constexpr Gp fp = x8;

} // {regs}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED