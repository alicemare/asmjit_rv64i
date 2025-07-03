// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/func.h"
#include "../core/ralocal_p.h"
#include "../riscv/riscvrapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::RISCVRAPass - Construction & Destruction
// ==============================================

RISCVRAPass::RISCVRAPass() noexcept : BaseRAPass() {
  _availableRegs.set(RegGroup::kGp, 0xFFFFFFFFu & ~RegMask(1 << Reg::kIdSp));
  _availableRegs.set(RegGroup::kVec, 0xFFFFFFFFu);
}

RISCVRAPass::~RISCVRAPass() noexcept {}

// riscv::RISCVRAPass - Overrides
// ==============================

void RISCVRAPass::onInit() noexcept {
  Base::onInit();
  _localAllocator.setScratchGpRegs(RegMask(1 << Gp::kIdT0) | RegMask(1 << Gp::kIdT1));
}

void RISCVRAPass::onDone() noexcept {
  Base::onDone();
}

Error RISCVRAPass::emitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  RAWorkReg* workReg = _ra->workRegById(workId);
  Reg dst = Reg::fromTypeAndId(workReg->regType(), dstPhysId);
  Reg src = Reg::fromTypeAndId(workReg->regType(), srcPhysId);
  return cc()->emit(Inst::kIdMv, dst, src);
}

Error RISCVRAPass::emitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  RAWorkReg* aWorkReg = _ra->workRegById(aWorkId);
  RAWorkReg* bWorkReg = _ra->workRegById(bWorkId);

  Reg a = Reg::fromTypeAndId(aWorkReg->regType(), aPhysId);
  Reg b = Reg::fromTypeAndId(bWorkReg->regType(), bPhysId);
  Reg t = Gp(Gp::kIdT0);

  cc()->emit(Inst::kIdXor, t, a, b);
  cc()->emit(Inst::kIdXor, a, t, a);
  cc()->emit(Inst::kIdXor, b, t, b);

  return kErrorOk;
}

Error RISCVRAPass::emitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  RAWorkReg* workReg = _ra->workRegById(workId);
  Reg dst = Reg::fromTypeAndId(workReg->regType(), dstPhysId);
  Mem src = workReg->spillMem();
  return cc()->emit(Inst::kIdLd, dst, src);
}

Error RISCVRAPass::emitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  RAWorkReg* workReg = _ra->workRegById(workId);
  Reg src = Reg::fromTypeAndId(workReg->regType(), srcPhysId);
  Mem dst = workReg->spillMem();
  return cc()->emit(Inst::kIdSd, src, dst);
}

Error RISCVRAPass::emitJump(const Label& label) noexcept {
  return cc()->jmp(label);
}

Error RISCVRAPass::emitPreCall(InvokeNode* invokeNode) noexcept {
  // TODO: Spill registers.
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER