// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVINSTDB_H_INCLUDED
#define ASMJIT_RISCV_RISCVINSTDB_H_INCLUDED

#include "../core/inst.h"
#include "../core/support.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \addtogroup asmjit_riscv
//! \{

namespace Inst {

//! Instruction IDs (RISC-V).
enum Id : uint32_t {
  kIdNone = 0,
  // R-Type
  kIdAdd,
  kIdAddw,
  kIdSub,
  kIdSubw,
  kIdSll,
  kIdSllw,
  kIdSrl,
  kIdSrlw,
  kIdSra,
  kIdSraw,
  kIdSlt,
  kIdSltu,
  kIdAnd,
  kIdOr,
  kIdXor,
  // I-Type
  kIdAddi,
  kIdAddiw,
  kIdSlti,
  kIdSltiu,
  kIdAndi,
  kIdOri,
  kIdXOri,
  kIdSlli,
  kIdSlliw,
  kIdSrli,
  kIdSrliw,
  kIdSrai,
  kIdSraiw,
  kIdJalr,
  // Load
  kIdLb,
  kIdLh,
  kIdLw,
  kIdLbu,
  kIdLhu,
  kIdLwu,
  kIdLd,
  // S-Type
  kIdSb,
  kIdSh,
  kIdSw,
  kIdSd,
  // B-Type
  kIdBeq,
  kIdBne,
  kIdBlt,
  kIdBge,
  kIdBltu,
  kIdBgeu,
  // U-Type
  kIdLui,
  kIdAuipc,
  // J-Type
  kIdJal,
  // Others
  kIdNop,
  kIdFence,
  // ... more instructions will be added here

  kIdCount
};

//! Tests whether the instruction `id` is a valid and known instruction.
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG bool isDefinedId(InstId id) noexcept {
  return id > 0 && id < kIdCount;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isRType(InstId id) {
  return id >= kIdAdd && id <= kIdXor;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isIType(InstId id) {
  return id >= kIdAddi && id <= kIdSraiw;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isSType(InstId id) {
  return id >= kIdSb && id <= kIdSd;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isBType(InstId id) {
  return id >= kIdBeq && id <= kIdJalr;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isUType(InstId id) {
  return id >= kIdLui && id <= kIdAuipc;
}

[[nodiscard]]
ASMJIT_INLINE_NODEBUG bool isJType(InstId id) {
  return id >= kIdJal && id <= kIdJal;
}

} // {Inst}

namespace InstDB {

//! Instruction encoding type (RISC-V).
enum class EncodingType : uint8_t {
  kNone = 0,
  kR,
  kI,
  kS,
  kB,
  kU,
  kJ
};

//! Instruction flags (RISC-V).
enum InstFlags : uint16_t {
  kIsLoad = 0x0001u,
  kIsStore = 0x0002u,
  kIsJump = 0x0004u,
  kIsBranch = 0x0008u
};

//! Instruction information (RISC-V).
struct InstInfo {
  //! Instruction encoding type.
  uint32_t _encoding : 8;
  //! Index to data specific to each encoding type.
  uint32_t _encodingDataIndex : 8;
  uint32_t _reserved : 16;

  uint16_t _rwInfoIndex;
  uint16_t _flags;

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rwInfoIndex() const noexcept { return _rwInfoIndex; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t flags() const noexcept { return _flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasFlag(uint32_t flag) const { return (_flags & flag) != 0; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EncodingType encoding() const noexcept { return (EncodingType)_encoding; }

  //! \}
};

ASMJIT_VARAPI const InstInfo _instInfoTable[];

[[nodiscard]]
static inline const InstInfo& infoById(InstId instId) noexcept {
  instId &= uint32_t(InstIdParts::kRealId);
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _instInfoTable[instId];
}

} // {InstDB}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVINSTDB_H_INCLUDED