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

  ASMJIT_INST_2x(adr, Adr, Gp, Label)
  ASMJIT_INST_2x(adr, Adr, Gp, Mem)

  inline Error jal(const Label& o0) { return _emitter()->_emitI(Inst::kIdJal, regs::ra, o0); }
  inline Error nop() { return _emitter()->_emitI(Inst::kIdNop); }
  inline Error fence() { return _emitter()->_emitI(Inst::kIdFence); }
  // some pseudo instructions
  inline Error mov(Gp dst, Gp src) { return _emitter()->_emitI(Inst::kIdAdd, dst, src, regs::zero); }
  // tmp workaroud 需要更多类型或者模版
  inline Error mov(Gp dst, Imm imm) { return li(dst, imm); }
  inline Error j(const Label& o0) { return _emitter()->_emitI(Inst::kIdJal, regs::zero, o0); }
  inline Error li(Gp dst, Imm imm) {
    int64_t value = imm.value();
    if (value >= -2048 && value <= 2047) {
        return _emitter()->_emitI(Inst::kIdAddi, dst, regs::zero, imm);
    }
    else if (value >= INT32_MIN && value <= INT32_MAX) {
        // 32位常量：LUI + ADDI
        uint32_t hi20 = (value + 0x800) >> 12;
        int32_t lo12 = value & 0xFFF;
        if (lo12 > 2047) lo12 -= 4096;
        _emitter()->_emitI(Inst::kIdLui, dst, Imm(hi20));
        if (lo12 != 0) {
            return _emitter()->_emitI(Inst::kIdAddi, dst, dst, Imm(lo12));
        }
        return kErrorOk;
    }
    else {
        // 64位常量：放入常量池，然后用AUIPC + LD加载
        // 这需要Compiler支持，暂时用多指令序列
        return loadImmediate64(dst, value);
    }
  }
  Error loadImmediate64(const Gp& dst, uint64_t value) {
    // 方案A：多步移位加载（6-8条指令）
    if ((value & 0xFFFFFFFF00000000ULL) == 0) {
        // 32位值
        return mov(dst, Imm(int32_t(value)));
    }
    // 真正的64位值：分块加载
    // 从高位开始，每次12位
    uint32_t parts[6];
    parts[5] = (value >> 60) & 0xF;    // 最高4位
    parts[4] = (value >> 48) & 0xFFF;  // 59:48
    parts[3] = (value >> 36) & 0xFFF;  // 47:36
    parts[2] = (value >> 24) & 0xFFF;  // 35:24
    parts[1] = (value >> 12) & 0xFFF;  // 23:12
    parts[0] = value & 0xFFF;          // 11:0
    // 找到最高非零部分
    int start = 5;
    while (start >= 0 && parts[start] == 0) start--;
    if (start < 0) {
        // 值为0
        return _emitter()->_emitI(Inst::kIdAddi, dst, regs::zero, Imm(0));
    }
    // 加载最高部分
    _emitter()->_emitI(Inst::kIdAddi, dst, regs::zero, Imm(parts[start]));
    // 依次加载其他部分
    for (int i = start - 1; i >= 0; i--) {
        _emitter()->_emitI(Inst::kIdSlli, dst, dst, Imm(12));
        if (parts[i] != 0) {
            _emitter()->_emitI(Inst::kIdOri, dst, dst, Imm(parts[i]));
        }
    }
    return kErrorOk;
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