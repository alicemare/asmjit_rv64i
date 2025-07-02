// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED
#define ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED

#include "../core/emitter.h"
#include "../core/support.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

#define ASMJIT_INST_2x(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID, o0, o1); }

#define ASMJIT_INST_3x(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2); }

//! \addtogroup asmjit_riscv
//! \{

//! Emitter (RISC-V).
template<typename This>
struct EmitterExplicitT {
  ASMJIT_INLINE_NODEBUG This* _emitter() noexcept { return static_cast<This*>(this); }
  ASMJIT_INLINE_NODEBUG const This* _emitter() const noexcept { return static_cast<const This*>(this); }

  // RV64I Base Instruction Set
  ASMJIT_INST_3x(add, Add, Gp, Gp, Gp)
  ASMJIT_INST_3x(addi, Addi, Gp, Gp, Imm)

  // ... more instructions will be added here
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED