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

Error EmitHelper::emitArgMove(
  const Reg& dst_, TypeId dstTypeId,
  const Operand_& src_, TypeId srcTypeId, const char* comment) {

  // Deduce optional `dstTypeId`, which may be `TypeId::kVoid` in some cases.
  if (dstTypeId == TypeId::kVoid) {
    dstTypeId = RegUtils::typeIdOf(dst_.regType());
  }

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::isValid(dstTypeId) && !TypeUtils::isAbstract(dstTypeId));
  ASMJIT_ASSERT(TypeUtils::isValid(srcTypeId) && !TypeUtils::isAbstract(srcTypeId));

  Reg dst(dst_.as<Reg>());
  Operand src(src_);

  uint32_t dstSize = TypeUtils::sizeOf(dstTypeId);

  if (TypeUtils::isInt(dstTypeId)) {
    if (TypeUtils::isInt(srcTypeId)) {
      uint32_t x = uint32_t(dstSize == 8);

      dst.setSignature(OperandSignature{x ? RegTraits<RegType::kGp64>::kSignature : RegTraits<RegType::kGp32>::kSignature});
      _emitter->setInlineComment(comment);

      if (src.isReg()) {
        src.setSignature(dst.signature());
        return _emitter->emit(Inst::kIdAdd, dst, src, regs::x0);
      }
      else if (src.isMem()) {
        InstId instId = Inst::kIdNone;
          switch (srcTypeId) {
          case TypeId::kInt8: instId = Inst::kIdLb; break;
          case TypeId::kUInt8: instId = Inst::kIdLbu; break;
          case TypeId::kInt16: instId = Inst::kIdLh; break;
          case TypeId::kUInt16: instId = Inst::kIdLhu; break;
          case TypeId::kInt32: instId = Inst::kIdLw; break;
          case TypeId::kUInt32: instId = Inst::kIdLwu; break;
          case TypeId::kInt64: instId = Inst::kIdLd; break;
          case TypeId::kUInt64: instId = Inst::kIdLd; break;
          default:
            return DebugUtils::errored(kErrorInvalidState);
        }
        return _emitter->emit(instId, dst, src);
      }
    }
  }

  return DebugUtils::errored(kErrorInvalidState);
}

Error EmitHelper::emitProlog(const FuncFrame& frame) {
  printf("=== PROLOG DEBUG ===\n");
  printf("stackAdjustment: %u\n", frame.stackAdjustment());
  printf("finalStackSize: %u\n", frame.finalStackSize());
  printf("pushPopSaveOffset: %u\n", frame.pushPopSaveOffset());
  printf("pushPopSaveSize: %u\n", frame.pushPopSaveSize());
  printf("localStackOffset: %u\n", frame.localStackOffset());
  printf("savedRegs mask: 0x%x\n", frame.savedRegs(RegGroup::kGp));

  using namespace regs;
  Emitter* emitter = _emitter->as<Emitter>();
  uint32_t stackSize = frame.finalStackSize();
  if (stackSize == 0) {
    return kErrorOk;
  }

  // Alloc stack space
  emitter->addi(sp, sp, -int32_t(stackSize));

  RegMask savedGpMask = frame.savedRegs(RegGroup::kGp);
  // offset is SP-relative
  int32_t offset = int32_t(frame.pushPopSaveOffset());

  // 1. 保存 ra (x1)
  if (savedGpMask & Support::bitMask(1)) {
    emitter->sd(ra, Mem(sp, offset));
    offset += 8;
  }

  // 2. 保存 fp/s0 (x8)
  if (savedGpMask & Support::bitMask(8)) {
    emitter->sd(fp, Mem(sp, offset));
    offset += 8;
  }

  // 3. 保存 s1 (x9)
  if (savedGpMask & Support::bitMask(9)) {
    emitter->sd(x9, Mem(sp, offset));
    offset += 8;
  }

  // 4. 保存 s2-s11 (x18-x27)
  for (uint32_t regId = 18; regId <= 27; regId++) {
    if (savedGpMask & Support::bitMask(regId)) {
      Gp reg = Gp::make_x(regId);
      emitter->sd(reg, Mem(sp, offset));
      offset += 8;
    }
  }

  // 如果使用帧指针，设置它
  if (frame.hasPreservedFP()) {
    emitter->addi(fp, sp, int32_t(stackSize));
  }

  return kErrorOk;
}

Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  printf("=== PROLOG DEBUG ===\n");
  printf("stackAdjustment: %u\n", frame.stackAdjustment());
  printf("finalStackSize: %u\n", frame.finalStackSize());
  printf("pushPopSaveOffset: %u\n", frame.pushPopSaveOffset());
  printf("pushPopSaveSize: %u\n", frame.pushPopSaveSize());
  printf("localStackOffset: %u\n", frame.localStackOffset());
  printf("savedRegs mask: 0x%x\n", frame.savedRegs(RegGroup::kGp));

  using namespace regs;
  Emitter* emitter = _emitter->as<Emitter>();
  uint32_t stackSize = frame.finalStackSize();
  if (stackSize == 0) {
    emitter->jalr(x0, ra, 0); // ret
    return kErrorOk;
  }

  RegMask savedGpMask = frame.savedRegs(RegGroup::kGp);
  // Use extraRegSaveOffset for restoration
  int32_t offset = int32_t(frame.pushPopSaveOffset());

  // 1. 恢复 ra (x1)
  if (savedGpMask & Support::bitMask(1)) {
    emitter->ld(regs::ra, Mem(sp, offset));
    offset += 8;
  }

  // 2. 恢复 fp/s0 (x8)
  if (savedGpMask & Support::bitMask(8)) {
    emitter->ld(regs::fp, Mem(sp, offset));
    offset += 8;
  }

  // 3. 恢复 s1 (x9)
  if (savedGpMask & Support::bitMask(9)) {
    emitter->ld(regs::x9, Mem(sp, offset));
    offset += 8;
  }

  // 4. 恢复 s2-s11 (x18-x27)
  for (uint32_t regId = 18; regId <= 27; regId++) {
    if (savedGpMask & Support::bitMask(regId)) {
      Gp reg = Gp::make_x(regId);
      emitter->ld(reg, Mem(sp, offset));
      offset += 8;
    }
  }

  emitter->addi(sp, sp, int32_t(stackSize));
  emitter->jalr(x0, ra, 0);
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