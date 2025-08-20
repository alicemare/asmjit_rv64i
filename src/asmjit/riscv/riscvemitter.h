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

  ASMJIT_INST_2x(sb, Sb, Gp, Mem)
  ASMJIT_INST_2x(sh, Sh, Gp, Mem)
  ASMJIT_INST_2x(sw, Sw, Gp, Mem)
  ASMJIT_INST_2x(sd, Sd, Gp, Mem)

  ASMJIT_INST_3x(beq, Beq, Gp, Gp, Label)
  ASMJIT_INST_3x(bne, Bne, Gp, Gp, Label)
  ASMJIT_INST_3x(blt, Blt, Gp, Gp, Label)
  ASMJIT_INST_3x(bge, Bge, Gp, Gp, Label)
  ASMJIT_INST_3x(bltu, Bltu, Gp, Gp, Label)
  ASMJIT_INST_3x(bgeu, Bgeu, Gp, Gp, Label)

  ASMJIT_INST_2x(lui, Lui, Gp, Imm)
  ASMJIT_INST_2x(auipc, Auipc, Gp, Imm)

  ASMJIT_INST_2x(jal, Jal, Gp, Label)

  //ASMJIT_INST_2x(adr, Adr, Gp, Label)
  //ASMJIT_INST_2x(adr, Adr, Gp, Mem)

//  inline Error ret() { return _emitter()->_emitI(Inst::kIdJalr, regs::x0, regs::ra, Imm(0)); }
  inline Error nop() { return _emitter()->_emitI(Inst::kIdNop); }
  inline Error fence() { return _emitter()->_emitI(Inst::kIdFence); }
  // some pseudo instructions
  inline Error mov(Gp dst, Gp src) { return _emitter()->_emitI(Inst::kIdAdd, dst, src, regs::zero); }
  // tmp workaroud 需要更多类型或者模版
  inline Error mov(Gp dst, Imm imm) { return li(dst, imm); }
  inline Error j(const Label& o0) { return _emitter()->_emitI(Inst::kIdJal, regs::zero, o0); }
  inline Error li(Gp dst, Imm imm) {
    uint32_t lo32 = imm.uint32Lo();
    uint32_t hi32 = imm.uint32Hi();
    uint64_t val = ((uint64_t)hi32 << 32) | lo32;
    const auto& zr = regs::x0;
    Error err = kErrorOk;

    if (val == 0) {
        return _emitter()->_emitI(Inst::kIdAdd, dst, zr, zr);
    }

    if ((int64_t)val >= -2048 && (int64_t)val < 2048) {
        return _emitter()->_emitI(Inst::kIdAddi, dst, zr, Imm(val));
    }

    if ((int64_t)val == (int32_t)val) {
        int32_t sval = (int32_t)val;
        
        // lui + addiw 组合
        int32_t hi20 = (sval + 0x800) >> 12;  // addiw need sign-extend
        int32_t lo12 = sval & 0xFFF;
        
        ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdLui, dst, Imm(hi20)));
        return _emitter()->_emitI(Inst::kIdAddiw, dst, dst, Imm(lo12));
    }

    // 情况3：完整64位数
    // 策略：使用最多8条指令，每次加载12位并移位

    // 将64位数看作 5.33 个12位段
    // 从高位到低位依次构建

    // 最简单的实现：分6次，每次处理11位（除了最后一次处理9位）
    // 总共 64 = 11*5 + 9 位

    bool first = true;

    // 处理 bit[63:53] (11 bits)
    uint64_t chunk = (val >> 53) & 0x7FF;
    if (chunk != 0 || first) {
      ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, zr, Imm(chunk)));
      first = false;
    }

    // 处理 bit[52:42] (11 bits)
    chunk = (val >> 42) & 0x7FF;
    if (!first) ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(11)));
    if (chunk != 0 || first) {
      ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, first ? zr : dst, Imm(chunk)));
      first = false;
    }

    // 处理 bit[41:31] (11 bits)
    chunk = (val >> 31) & 0x7FF;
    if (!first) ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(11)));
    if (chunk != 0 || first) {
      ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, first ? zr : dst, Imm(chunk)));
      first = false;
    }

    // 处理 bit[30:20] (11 bits)
    chunk = (val >> 20) & 0x7FF;
    if (!first) ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(11)));
    if (chunk != 0 || first) {
      ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, first ? zr : dst, Imm(chunk)));
      first = false;
    }

    // 处理 bit[19:9] (11 bits)
    chunk = (val >> 9) & 0x7FF;
    if (!first) ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(11)));
    if (chunk != 0 || first) {
      ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, first ? zr : dst, Imm(chunk)));
      first = false;
    }

    // 处理 bit[8:0] (9 bits)
    chunk = val & 0x1FF;
    if (!first) ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(9)));
    ASMJIT_PROPAGATE(_emitter()->_emitI(Inst::kIdAddi, dst, first ? zr : dst, Imm(chunk)));

    return err;
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