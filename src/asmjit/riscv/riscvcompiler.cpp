// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

// #include "../riscv/riscvassembler.h"
#include "../riscv/riscvcompiler.h"
#include "riscvemithelper_p.h"
#include "../riscv/riscvassembler.h"
#include "../riscv/riscvrapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::Compiler - Construction & Destruction
// ============================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  _archMask = uint64_t(1) << uint32_t(Arch::kRISCV64);
  _environment.setArch(Arch::kRISCV64);
  initEmitterFuncs(this);

  if (code)
    code->attach(this);
}

Compiler::~Compiler() noexcept {}

// riscv::Compiler - Overrides
// ===========================

Error Compiler::onAttach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));
  initEmitterFuncs(this);

  Error err = addPassT<RISCVRAPass>();
  if (err) {
    onDetach(code);
    return err;
  }

  return kErrorOk;
}

Error Compiler::onDetach(CodeHolder& code) noexcept {
  return Base::onDetach(code);
}

Error Compiler::finalize() {
  ASMJIT_PROPAGATE(runPasses());

  Assembler a(_code);
  a.addEncodingOptions(encodingOptions());

  return serializeTo(&a);
}

ASMJIT_INLINE_NODEBUG Error Compiler::loadAddressOf(const Gp& dst, const Mem& mem) {
    Gp dstX = Gp::make_x(dst.id());
    // dst 必须是 x 寄存器
    if (!mem.hasBaseReg()) {
    // 情况1：纯立即数偏移 [offset]
    int64_t offset = mem.offset();
    
    if (offset >= -2048 && offset <= 2047) {
      // li dst, imm (小立即数，生成 addi dst, x0, imm)
      return addi(dstX, regs::zero, static_cast<int32_t>(offset));
    } else {
      // 大立即数，使用 LUI + ADDI 序列
      int32_t lo12 = static_cast<int32_t>(offset) & 0xFFF;
      if (lo12 & 0x800) {
        lo12 |= 0xFFFFF000;  // 符号扩展12位到32位
      }
      int32_t hi20 = (static_cast<int32_t>(offset) - lo12) >> 12;
        
      if (lo12 == 0) {
        // 只需要 LUI
        return lui(dstX, hi20);
      }
      else {
        // LUI + ADDI 序列
        ASMJIT_PROPAGATE(lui(dstX, hi20));
        return addi(dstX, dstX, lo12);
      }
    }
  } else {
    // 情况2：基址寄存器 + 偏移量 [base + offset]
    if (mem.hasIndex()) {
      // RISC-V 不支持 [base + index + offset] 寻址模式
      return DebugUtils::errored(kErrorInvalidAddressIndex);
    }
    
    Gp baseReg = Gp::make_x(mem.baseId());  // 确保基址寄存器是64位
    int64_t offset = mem.offset();
    
    if (offset == 0) {
      // mv dst, base (实际生成 addi dst, base, 0)
      return addi(dstX, baseReg, 0);
    }
    else if (offset >= -2048 && offset <= 2047) {
      // addi dst, base, offset
      return addi(dstX, baseReg, static_cast<int32_t>(offset));
    }
    else {
      // 大偏移量，需要使用临时寄存器或多条指令
      int32_t lo12 = static_cast<int32_t>(offset) & 0xFFF;
      if (lo12 & 0x800) {
        lo12 |= 0xFFFFF000;  // 符号扩展12位到32位
      }
      int32_t hi20 = (static_cast<int32_t>(offset) - lo12) >> 12;
      
      if (dstX.id() != baseReg.id()) {
        // 目标寄存器与基址寄存器不同，可以直接使用目标寄存器作为临时寄存器
        if (lo12 == 0) {
          // lui dst, hi20; add dst, dst, base
          ASMJIT_PROPAGATE(lui(dstX, hi20));
          return add(dstX, dstX, baseReg);
        }
        else {
          // lui dst, hi20; add dst, dst, base; addi dst, dst, lo12
          ASMJIT_PROPAGATE(lui(dstX, hi20));
          ASMJIT_PROPAGATE(add(dstX, dstX, baseReg));
          return addi(dstX, dstX, lo12);
        }
      } else {
        // [[TODO]]
        return DebugUtils::errored(kErrorInvalidState);
      }
    }
  }
}


ASMJIT_INLINE_NODEBUG Error Compiler::loadAddressOf(const Gp& o0, const Label& o1) {
  // [[TODO]]
}


ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER