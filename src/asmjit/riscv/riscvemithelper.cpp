// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-config.h"
#include "riscvemithelper_p.h"

#include "../core/formatter.h"
#include "../core/funcargscontext_p.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../riscv/riscvemithelper_p.h"
#include "../riscv/riscvoperand.h"
#include "../riscv/riscvformatter_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

Error EmitHelper::emitRegMove(const Operand_& dst_, const Operand_& src_, TypeId typeId, const char* comment) {
  Emitter* emitter = _emitter->as<Emitter>();
  emitter->setInlineComment(comment);

  if (dst_.isReg() && src_.isMem()) {
    Reg dst(dst_.as<Reg>());
    Mem src(src_.as<Mem>());

    switch (typeId) {
      case TypeId::kInt8:
        return emitter->lb(dst.as<Gp>(), src);
      case TypeId::kUInt8:
        return emitter->lbu(dst.as<Gp>(), src);
      case TypeId::kInt16:
        return emitter->lh(dst.as<Gp>(), src);
      case TypeId::kUInt16:
        return emitter->lhu(dst.as<Gp>(), src);
      case TypeId::kInt32:
        return emitter->lw(dst.as<Gp>(), src);
      case TypeId::kUInt32:
        return emitter->lwu(dst.as<Gp>(), src);
      case TypeId::kInt64:
      case TypeId::kUInt64:
        return emitter->ld(dst.as<Gp>(), src);
      default:
        break;
    }
  }

  if (dst_.isMem() && src_.isReg()) {
    Mem dst(dst_.as<Mem>());
    Reg src(src_.as<Reg>());

    switch (typeId) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
        return emitter->sb(src.as<Gp>(), dst);
      case TypeId::kInt16:
      case TypeId::kUInt16:
        return emitter->sh(src.as<Gp>(), dst);
      case TypeId::kInt32:
      case TypeId::kUInt32:
        return emitter->sw(src.as<Gp>(), dst);
      case TypeId::kInt64:
      case TypeId::kUInt64:
        return emitter->sd(src.as<Gp>(), dst);
      default:
        break;
    }
  }

  if (dst_.isReg() && src_.isReg()) {
    return emitter->mov(dst_.as<Gp>(), src_.as<Gp>());
  }

  emitter->setInlineComment(nullptr);
  return DebugUtils::errored(kErrorInvalidState);
}

Error EmitHelper::emitRegSwap(const Reg& a, const Reg& b, const char* comment) {
  DebugUtils::unused(a, b, comment);
  return DebugUtils::errored(kErrorInvalidState);
}

Error EmitHelper::emitArgMove(const Reg& dst_, TypeId dstTypeId, const Operand_& src_, TypeId srcTypeId, const char* comment) {
  // TODO:
  return kErrorOk;
}

Error EmitHelper::emitProlog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  const Gp& sp = regs::sp;
  const Gp& fp = regs::fp;
  const Gp& ra = regs::ra;
  int32_t offset = 0;

  uint32_t stackAdjustment = frame._stackAdjustment;
  if (stackAdjustment) {
    emitter->addi(sp, sp, -int32_t(stackAdjustment));
    emitter->sd(ra, Mem(sp, offset));
    offset += 8;
  }

  if (frame.hasPreservedFP()) {
    emitter->sd(fp, Mem(sp, offset));
    offset += 8;
    // emitter->addi(fp, sp, int32_t(frame.finalStackSize()));
  }

  return kErrorOk;
}

Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  const Gp& sp = regs::sp;
  const Gp& fp = regs::fp;
  const Gp& ra = regs::ra;
  
  uint32_t stackAdjustment = frame.stackAdjustment();
  int32_t offset = 0;

  if (stackAdjustment) {
    emitter->ld(ra, Mem(sp, offset));
    offset += 8;
  }
  if (frame.hasPreservedFP()) {
    emitter->ld(fp, Mem(sp, offset));
    offset += 8;
  }

  if (stackAdjustment) {
    emitter->addi(sp, sp, int32_t(stackAdjustment));
  }

  emitter->jalr(ra, ra, 0); //ret
  return kErrorOk;
}

static Error ASMJIT_CDECL Emitter_emitProlog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitProlog(frame);
}

static Error ASMJIT_CDECL Emitter_emitEpilog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitEpilog(frame);
}

static Error ASMJIT_CDECL Emitter_emitArgsAssignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitArgsAssignment(frame, args);
}

void initEmitterFuncs(BaseEmitter* emitter) {
  emitter->_funcs.emitProlog = Emitter_emitProlog;
  emitter->_funcs.emitEpilog = Emitter_emitEpilog;
  emitter->_funcs.emitArgsAssignment = Emitter_emitArgsAssignment;
#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.formatInstruction = FormatterInternal::formatInstruction;
#endif

}

ASMJIT_END_SUB_NAMESPACE