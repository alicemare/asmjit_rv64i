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

  //! \name Constants
  //! \{

  //! Special register id.
  enum Id : uint32_t {
    kIdZr = 0,      //!< Zero register `zero`.
    kIdRa = 1,      //!< Return address `ra`.
    kIdSp = 2,      //!< Stack pointer `sp`.
    kIdGp = 3,      //!< Global pointer `gp`.
    kIdTp = 4,      //!< Thread pointer `tp`.
    kIdFp = 8       //!< Frame pointer `fp` (s0).
  };

  //! \}

  //! \name Static Constructors
  //! \{

  //! Creates a new 32-bit GP register (W) having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_w(uint32_t regId) noexcept { return Gp(_signatureOf<RegType::kGp32>(), regId); }

  //! Creates a new 64-bit GP register (X) having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_x(uint32_t regId) noexcept { return Gp(_signatureOf<RegType::kGp64>(), regId); }

  //! \}
};

//! Vector register (RISC-V).
class Vec : public UniVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  // TODO: RISC-V vector registers implementation.
};

//! Memory operand (RISC-V).
//! RV64I only have one addressing type: [reg + imm]
//! rs1 base register  + 12bit immediate offset
//! Load, I-Type
//! +-----------+-------+-------+-------+----------+
//! | imm[11:0] | rs1   | funct3| rd    | opcode   |
//! +-----------+-------+-------+-------+----------+
//! 31        20 19    15 14   12 11   7 6         0
//! Store, S-Type
//! +-----------+-------+-------+-------+----------+----------+
//! | imm[11:5] | rs2   | rs1   | funct3| imm[4:0] | opcode   |
//! +-----------+-------+-------+-------+----------+----------+
//! 31        25 24  20 19    15 14   12 11       7 6         0
class Mem : public BaseMem {
public:
  enum AdditionalBits : uint32_t {
    // Nothing specifit to RV64I
  };
  //! \name Construction & Destruction
  //! \{

  //! Creates a default memory operand that points to [0].
  ASMJIT_INLINE_CONSTEXPR Mem() noexcept : BaseMem() {}
  //! Creates a copy of the `other` memory operand.
  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept : BaseMem(other) {}
  //! Creates a memory operand based on `baseReg` and `offset`.
  ASMJIT_INLINE_CONSTEXPR Mem(const Gp& baseReg, int32_t offset = 0) noexcept : BaseMem(baseReg, offset) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
  : BaseMem(Globals::NoInit) {}

  // NOTE: RISC-V doesn't have [base + index + offset] addressing mode.

  //! \}

  //! \name Base & Offset
  //! \{

  //! Converts memory `baseType` and `baseId` to `riscv::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg baseReg() const noexcept { return Reg::fromTypeAndId(baseType(), baseId()); }
  //! \}

  //! \name
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem& operator=(const Mem& other) noexcept { copyFrom(other); return *this; }

  static constexpr Mem ptr(const Gp& baseReg, int32_t offset = 0) noexcept { return Mem(baseReg, offset); }

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

//! Register IDs.
//!
//! \note Register IDs are compatible with the RISC-V specification.
enum RegId : uint32_t {
  kIdZero = 0,    //!< Zero register (x0).
  kIdRa   = 1,    //!< Return address register (x1).
  kIdSp   = 2,    //!< Stack pointer register (x2).
  kIdGp   = 3,    //!< Global pointer register (x3).
  kIdTp   = 4,    //!< Thread pointer register (x4).
  kIdT0   = 5,    //!< Temporary register (x5).
  kIdT1   = 6,    //!< Temporary register (x6).
  kIdT2   = 7,    //!< Temporary register (x7).
  kIdFp   = 8,    //!< Frame pointer register (x8) / Saved register (s0).
  kIdS1   = 9,    //!< Saved register (x9).
  kIdA0   = 10,   //!< Function argument / return value register (x10).
  kIdA1   = 11,   //!< Function argument / return value register (x11).
  kIdA2   = 12,   //!< Function argument register (x12).
  kIdA3   = 13,   //!< Function argument register (x13).
  kIdA4   = 14,   //!< Function argument register (x14).
  kIdA5   = 15,   //!< Function argument register (x15).
  kIdA6   = 16,   //!< Function argument register (x16).
  kIdA7   = 17,   //!< Function argument register (x17).
  kIdS2   = 18,   //!< Saved register (x18).
  kIdS3   = 19,   //!< Saved register (x19).
  kIdS4   = 20,   //!< Saved register (x20).
  kIdS5   = 21,   //!< Saved register (x21).
  kIdS6   = 22,   //!< Saved register (x22).
  kIdS7   = 23,   //!< Saved register (x23).
  kIdS8   = 24,   //!< Saved register (x24).
  kIdS9   = 25,   //!< Saved register (x25).
  kIdS10  = 26,   //!< Saved register (x26).
  kIdS11  = 27,   //!< Saved register (x27).
  kIdT3   = 28,   //!< Temporary register (x28).
  kIdT4   = 29,   //!< Temporary register (x29).
  kIdT5   = 30,   //!< Temporary register (x30).
  kIdT6   = 31,   //!< Temporary register (x31).

  //! Count of registers.
  kRegCount = 32
};

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED