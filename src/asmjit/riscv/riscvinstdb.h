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
  kIdAdd,
  kIdAddi,
  // ... more instructions will be added here

  kIdCount
};

} // {Inst}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVINSTDB_H_INCLUDED