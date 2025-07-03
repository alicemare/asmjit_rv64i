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
#include "../riscv/riscvformatter_p.h"
#include "../riscv/riscvinstapi_p.h"
#include "../riscv/riscvoperand.h"

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
    return emitter->mv(dst_.as<Gp>(), src_.as<Gp>());
  }

  emitter->setInlineComment(nullptr);
  return DebugUtils::errored(kErrorInvalidState);
}

Error EmitHelper::emitRegSwap(const Reg& a, const Reg& b, const char* comment) {
  Emitter* emitter = _emitter->as<Emitter>();
  emitter->setInlineComment(comment);

  Gp t = emitter->newTmpGp();
  emitter->mv(t, a.as<Gp>());
  emitter->mv(a.as<Gp>(), b.as<Gp>());
  emitter->mv(b.as<Gp>(), t);

  emitter->setInlineComment(nullptr);
  return kErrorOk;
}

Error EmitHelper::emitArgMove(const Reg& dst_, TypeId dstTypeId, const Operand_& src_, TypeId srcTypeId, const char* comment) {
  // TODO:
  return kErrorOk;
}

Error EmitHelper::emitProlog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  const Gp& sp = riscv::sp;
  const Gp& fp = riscv::fp;

  int stackSize = frame.stackSize();
  if (stackSize) {
    emitter->addi(sp, sp, -stackSize);
  }

  if (frame.hasPreservedFP()) {
    emitter->sd(riscv::ra, Mem(sp, frame.raOffset()));
    emitter->sd(fp, Mem(sp, frame.fpOffset()));
    emitter->addi(fp, sp, stackSize);
  }

  return kErrorOk;
}

Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();
  const Gp& sp = riscv::sp;
  const Gp& fp = riscv::fp;

  if (frame.hasPreservedFP()) {
    emitter->ld(riscv::ra, Mem(sp, frame.raOffset()));
    emitter->ld(fp, Mem(sp, frame.fpOffset()));
  }

  int stackSize = frame.stackSize();
  if (stackSize) {
    emitter->addi(sp, sp, stackSize);
  }

  emitter->ret();
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
}

ASMJIT_END_SUB_NAMESPACE