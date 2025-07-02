// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVARCHTRAITS_P_H_INCLUDED
#define ASMJIT_RISCV_RISCVARCHTRAITS_P_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/misc_p.h"
#include "../core/type.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \cond INTERNAL
//! \addtogroup asmjit_riscv
//! \{

static const constexpr ArchTraits riscvArchTraits = {
  // SP/FP/LR/PC.
  // x2 (sp), x8 (s0/fp), x1 (ra), 没有专门的 PC 寄存器
  Gp::kIdSp, Gp::kIdFp, Gp::kIdRa, 0xFFu,

  // Reserved.
  { 0u, 0u, 0u },

  // HW stack alignment (RISC-V requires stack aligned to 16 bytes at HW level).
  16u,

  // Min/max stack offset
  // RISC-V RV64I 支持的立即数范围为 -2048 到 2047（12位有符号立即数）
  2048, 2047,

  // Supported register types.
  0u | (1u << uint32_t(RegType::kGp32))
     | (1u << uint32_t(RegType::kGp64)),

  // Instruction hints [Gp, Vec, Mask, Extra].
  {{
    InstHints::kNoHints,    // No Push/Pop Hnit
    InstHints::kNoHints,    // No Vec Hint
    InstHints::kNoHints,    // No Mask Hint
    InstHints::kNoHints     // No Extra Hint
  }},

  // TypeIdToRegType.
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt64)   ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt64)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kGp64   : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V

  // Word names of 8-bit, 16-bit, 32-bit, and 64-bit quantities.
  {
    ArchTypeNameId::kByte,
    ArchTypeNameId::kHalf,
    ArchTypeNameId::kWord,
    ArchTypeNameId::kDWord
  }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVARCHTRAITS_P_H_INCLUDED
