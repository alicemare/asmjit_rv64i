// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED
#define ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED

#include "../core/emitter.h"
#include "../core/support.h"
#include "../riscv/riscvoperand.h"
#include "../riscv/riscvinstdb.h"

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
  ASMJIT_INST_3x(addw, Addw, Gp, Gp, Gp)
  ASMJIT_INST_3x(sub, Sub, Gp, Gp, Gp)
  ASMJIT_INST_3x(subw, Subw, Gp, Gp, Gp)
  ASMJIT_INST_3x(sll, Sll, Gp, Gp, Gp)
  ASMJIT_INST_3x(sllw, Sllw, Gp, Gp, Gp)
  ASMJIT_INST_3x(srl, Srl, Gp, Gp, Gp)
  ASMJIT_INST_3x(srlw, Srlw, Gp, Gp, Gp)
  ASMJIT_INST_3x(sra, Sra, Gp, Gp, Gp)
  ASMJIT_INST_3x(sraw, Sraw, Gp, Gp, Gp)
  ASMJIT_INST_3x(slt, Slt, Gp, Gp, Gp)
  ASMJIT_INST_3x(sltu, Sltu, Gp, Gp, Gp)
  ASMJIT_INST_3x(and_, And, Gp, Gp, Gp)
  ASMJIT_INST_3x(or_, Or, Gp, Gp, Gp)
  ASMJIT_INST_3x(xor_, Xor, Gp, Gp, Gp)

  ASMJIT_INST_3x(addi, Addi, Gp, Gp, Imm)
  ASMJIT_INST_3x(addiw, Addiw, Gp, Gp, Imm)
  ASMJIT_INST_3x(slti, Slti, Gp, Gp, Imm)
  ASMJIT_INST_3x(sltiu, Sltiu, Gp, Gp, Imm)
  ASMJIT_INST_3x(andi, Andi, Gp, Gp, Imm)
  ASMJIT_INST_3x(ori, Ori, Gp, Gp, Imm)
  ASMJIT_INST_3x(xori, XOri, Gp, Gp, Imm)
  ASMJIT_INST_3x(slli, Slli, Gp, Gp, Imm)
  ASMJIT_INST_3x(slliw, Slliw, Gp, Gp, Imm)
  ASMJIT_INST_3x(srli, Srli, Gp, Gp, Imm)
  ASMJIT_INST_3x(srliw, Srliw, Gp, Gp, Imm)
  ASMJIT_INST_3x(srai, Srai, Gp, Gp, Imm)
  ASMJIT_INST_3x(sraiw, Sraiw, Gp, Gp, Imm)
  ASMJIT_INST_3x(jalr, Jalr, Gp, Gp, Imm)

  ASMJIT_INST_2x(lb, Lb, Gp, Mem)
  ASMJIT_INST_2x(lh, Lh, Gp, Mem)
  ASMJIT_INST_2x(lw, Lw, Gp, Mem)
  ASMJIT_INST_2x(lbu, Lbu, Gp, Mem)
  ASMJIT_INST_2x(lhu, Lhu, Gp, Mem)
  ASMJIT_INST_2x(lwu, Lwu, Gp, Mem)
  ASMJIT_INST_2x(ld, Ld, Gp, Mem)

  ASMJIT_INST_2x(sb, Sb, Mem, Gp)
  ASMJIT_INST_2x(sh, Sh, Mem, Gp)
  ASMJIT_INST_2x(sw, Sw, Mem, Gp)
  ASMJIT_INST_2x(sd, Sd, Mem, Gp)

  ASMJIT_INST_3x(beq, Beq, Gp, Gp, Label)
  ASMJIT_INST_3x(bne, Bne, Gp, Gp, Label)
  ASMJIT_INST_3x(blt, Blt, Gp, Gp, Label)
  ASMJIT_INST_3x(bge, Bge, Gp, Gp, Label)
  ASMJIT_INST_3x(bltu, Bltu, Gp, Gp, Label)
  ASMJIT_INST_3x(bgeu, Bgeu, Gp, Gp, Label)

  ASMJIT_INST_2x(lui, Lui, Gp, Imm)
  ASMJIT_INST_2x(auipc, Auipc, Gp, Imm)

  ASMJIT_INST_2x(jal, Jal, Gp, Label)

  inline Error jal(const Label& o0) { return _emitter()->_emitI(Inst::kIdJal, regs::ra, o0); }
  inline Error nop() { return _emitter()->_emitI(Inst::kIdNop); }
  inline Error fence() { return _emitter()->_emitI(Inst::kIdFence); }
  // some pseudo instructions
  inline Error mv(Gp dst, Gp src) { return _emitter()->_emitI(Inst::kIdAdd, dst, src, regs::zero); }
  inline Error j(const Label& o0) { return _emitter()->_emitI(Inst::kIdJal, regs::zero, o0); }
  inline Error li(Gp dst, Imm imm) {
    if (imm.value() >= -2048 && imm.value() <= 2047) {
      return _emitter()->_emitI(Inst::kIdAddi, dst, regs::zero, imm);
    } else { // 对于大立即数，需要 lui+addi
      uint64_t value = static_cast<uint64_t>(imm.value());
      uint32_t hi = static_cast<uint32_t>((value + 0x800) >> 12) & 0xFFFFF;
      int32_t lo = static_cast<int32_t>(value & 0xFFF);
      _emitter()->_emitI(Inst::kIdLui, dst, Imm(hi));
      return _emitter()->_emitI(Inst::kIdAuipc, dst, dst, Imm(lo));
    }
  }

  // ... more instructions will be added here
};

//!
//! \note paste from a64emitter.h Emitter.
class Emitter : public BaseEmitter, public EmitterExplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};


//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVEMITTER_H_INCLUDED