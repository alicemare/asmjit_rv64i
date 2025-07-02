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


//! General purpose register (RISC-V).
class Gp : public UniGp {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)

  //! Special register id.
  enum Id : uint32_t {
    kIdZr = 0,      //!< Zero register `zero`.
    kIdRa = 1,      //!< Return address `ra`.
    kIdSp = 2,      //!< Stack pointer `sp`.
    kIdGp = 3,      //!< Global pointer `gp`.
    kIdTp = 4,      //!< Thread pointer `tp`.
    kIdFp = 8       //!< Frame pointer `fp` (s0).
  };

  //! Creates a new 32-bit GP register (W) having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_w(uint32_t regId) noexcept { return Gp(_signatureOf<RegType::kGp32>(), regId); }

  //! Creates a new 64-bit GP register (X) having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_x(uint32_t regId) noexcept { return Gp(_signatureOf<RegType::kGp64>(), regId); }

  //! Clones and casts this register to a 32-bit (W) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp w() const noexcept { return make_w(id()); }

  //! Clones and casts this register to a 64-bit (X) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp x() const noexcept { return make_x(id()); }
};

//! Vector register (RISC-V).
class Vec : public UniVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  // TODO: RISC-V vector registers implementation.
};

//! Memory operand (RISC-V).
class Mem : public BaseMem {
public:
  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem() noexcept : BaseMem() {}
  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept : BaseMem(other) {}
  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t baseId, uint32_t indexId, int32_t offset) noexcept
    : BaseMem(signature, baseId, indexId, offset) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Label& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(RegType::kLabelTag) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Gp& base, int64_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.regType()) |
              signature, base.id(), 0, int32_t(off)) {}

  // NOTE: RISC-V doesn't have [base + index] addressing mode.

  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t abs, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              signature, uint32_t(abs >> 32), 0, int32_t(uint32_t(abs & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem& operator=(const Mem& other) noexcept { copyFrom(other); return *this; }

  //! \}

  //! \name Clone
  //! \{

  //! Clones the memory operand.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }

  //! \}
};

namespace regs {

//! Creates a `w` register operand.
static ASMJIT_INLINE_NODEBUG constexpr Gp w(uint32_t id) noexcept { return Gp::make_w(id); }

//! Creates a `x` register operand.
static ASMJIT_INLINE_NODEBUG constexpr Gp x(uint32_t id) noexcept { return Gp::make_x(id); }

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